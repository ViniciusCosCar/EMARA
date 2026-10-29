
#include "freertos/FreeRTOS.h"

#include "app_wifi.h"
#include "esp_event.h"
#include "esp_mac.h"
#include "esp_wifi.h"

#include "utils.h"

// logging tag
static const char* TAG = "WiFi APsta";

// DHCP server option
#define DHCPS_OFFER_DNS 0x02

// network interfaces
esp_netif_t* sta_netif;
esp_netif_t* ap_netif;

void softap_set_dns_addr(esp_netif_t* esp_netif_ap, esp_netif_t* esp_netif_sta)
{
        esp_netif_dns_info_t dns;
        esp_netif_get_dns_info(esp_netif_sta, ESP_NETIF_DNS_MAIN, &dns);
        uint8_t dhcps_offer_option = DHCPS_OFFER_DNS;
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(esp_netif_ap));
        ESP_ERROR_CHECK(esp_netif_dhcps_option(esp_netif_ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER,
                                               &dhcps_offer_option, sizeof(dhcps_offer_option)));
        ESP_ERROR_CHECK(esp_netif_set_dns_info(esp_netif_ap, ESP_NETIF_DNS_MAIN, &dns));
        ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(esp_netif_ap));
}

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
        static bool nat_enabled = false;

        if (event_base == WIFI_EVENT)
        {
                switch (event_id)
                {
                // sta
                /* triggered by esp_wifi_start() returning ESP_OK and the current Wi-Fi mode
                 * configured to STA or STA/AP. Upon receiving this event, the event task
                 * will initialize the LwIP network interface (netif)
                 */
                case WIFI_EVENT_STA_START:
                        // connect to configured AP
                        LOGFI(TAG, "wifi driver started");
                        esp_wifi_connect();
                        break;

                /* If esp_wifi_connect() returns ESP_OK and the station successfully
                 * connects to the target AP, the connection event will arise. Upon
                 * receiving this event, the event task starts the DHCP client and begins
                 * the DHCP process of getting the IP address */
                case WIFI_EVENT_STA_CONNECTED:

                        LOGFI(TAG,
                              "connected to target AP. Starting DHCP client and the process of getting the IP address");
                        break;

                case WIFI_EVENT_STA_DISCONNECTED:
                {
                        /* TODO: before trying to reconnect, verify if WIFI_EVENT_STA_DISCONNECT
                         * was caused by esp_wifi_disconnect() or esp_wifi_stop() */
                        /* desperatly tries to reconnect */
                        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*)event_data;

                        LOGFE(TAG, "disconnected from AP. Trying to reconnect... rc=%d", event->reason);
                        esp_wifi_connect();
                        break;
                }
                // ap
                case WIFI_EVENT_AP_STACONNECTED:
                {
                        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*)event_data;
                        LOGFI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
                        break;
                }

                case WIFI_EVENT_AP_STADISCONNECTED:
                {
                        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*)event_data;
                        LOGFI(TAG, "station " MACSTR " leave, AID=%d, reason=%d", MAC2STR(event->mac), event->aid,
                              event->reason);
                        break;
                }
                }
                return;
        }
        if (event_base == IP_EVENT)
        {
                switch (event_id)
                {
                // sta
                /* if esp_wifi_connect() returns ESP_OK and the station successfully
                 * connects to the target AP, the connection event will arise. Upon
                 * receiving this event, the event task starts the DHCP client and begins
                 * the DHCP process of getting the IP address. This event arises when the
                 * DHCP client successfully gets the IPV4 address from the DHCP server, or
                 * when the IPV4 address is changed. The event means that everything is
                 * ready and the application can begin its tasks (e.g., creating sockets).
                 */
                case IP_EVENT_STA_GOT_IP:
                {
                        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;

                        LOGFI(TAG, "STA got ip: " IPSTR, IP2STR(&event->ip_info.ip));

                        // provide DNS to clients connected to the SoftAP
                        softap_set_dns_addr(ap_netif, sta_netif);

                        // use sta interface for outbound traffic
                        ESP_ERROR_CHECK(esp_netif_set_default_netif(sta_netif));

                        if (!nat_enabled)
                        {
                                // enable NAT for SoftAP clients.
                                esp_err_t err = esp_netif_napt_enable(ap_netif);
                                if (err != ESP_OK)
                                {
                                        ESP_LOGE(TAG, "Failed to enable NAPT: %s", esp_err_to_name(err));
                                }
                                else
                                        nat_enabled = true;
                        }

                        break;
                }
                // ap
                case IP_EVENT_ASSIGNED_IP_TO_CLIENT:
                {
                        const ip_event_assigned_ip_to_client_t* e = (const ip_event_assigned_ip_to_client_t*)event_data;
                        ESP_LOGI(TAG, "Assigned IP to client: " IPSTR ", MAC=" MACSTR ", hostname='%s'", IP2STR(&e->ip),
                                 MAC2STR(e->mac), e->hostname);
                }
                }
        }
}

// wifi ap + station initialization
esp_err_t wifi_init_apsta(void)
{
        // initialize TCP/IP network interface
        ESP_ERROR_CHECK(esp_netif_init());

        // start running event loop on backgroud
        ESP_ERROR_CHECK(esp_event_loop_create_default());

        // create both network interfaces
        sta_netif = esp_netif_create_default_wifi_sta();
        ap_netif  = esp_netif_create_default_wifi_ap();
        assert(sta_netif);
        assert(ap_netif);

        // intialize wifi
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));

        // register events
        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
        ESP_ERROR_CHECK(
            esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

        // config wifi and set mode to apsta
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
        wifi_config_t ap_config = {
            .ap =
                {
                     .ssid           = CONFIG_AP_WIFI_SSID,
                     .password       = CONFIG_AP_WIFI_PASSWORD,
                     .max_connection = CONFIG_AP_WIFI_MAX_CONNECTED,
                     .authmode       = WIFI_AUTH_WPA2_PSK,
                     },
        };

        wifi_config_t sta_config = {
            .sta =
                {
                      .ssid     = CONFIG_STA_WIFI_SSID,
                      .password = CONFIG_STA_WIFI_PASSWORD,
                      },
        };
        // set wifi configuration
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));

        // start wifi
        ESP_ERROR_CHECK(esp_wifi_start());

        LOGFI(TAG, "finished");
        LOGFI(TAG, "connect to AP SSID: %s Password: %s", CONFIG_AP_WIFI_SSID, CONFIG_AP_WIFI_PASSWORD);
        LOGFI(TAG, "ESP32 will attempt to connect to STA SSID: %s", CONFIG_STA_WIFI_SSID);

        return ESP_OK;
}
