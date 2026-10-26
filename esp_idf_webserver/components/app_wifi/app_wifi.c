
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "app_wifi.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "sdkconfig.h"
#include "utils.h"

// logging tag
static const char* s_TAG = "wifi station application";
bool wifi_connected      = false;
esp_netif_ip_info_t ip_info;

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
        if (event_base == WIFI_EVENT)
        {
                switch (event_id)
                {
                /* triggered by esp_wifi_start() returning ESP_OK and the current Wi-Fi mode
                 * configured to STA or STA/AP. Upon receiving this event, the event task
                 * will initialize the LwIP network interface (netif)
                 */
                case WIFI_EVENT_STA_START:
                        /* connect to configured AP */
                        LOGFI(s_TAG, "wifi driver started");
                        esp_wifi_connect();
                        break;

                /* If esp_wifi_connect() returns ESP_OK and the station successfully
                 * connects to the target AP, the connection event will arise. Upon
                 * receiving this event, the event task starts the DHCP client and begins
                 * the DHCP process of getting the IP address */
                case WIFI_EVENT_STA_CONNECTED:

                        LOGFI(s_TAG,
                              "connected to target AP. Starting DHCP client and the process of getting the IP address");
                        break;

                case WIFI_EVENT_STA_DISCONNECTED:
                        /* TODO: before trying to reconnect, verify if WIFI_EVENT_STA_DISCONNECT
                         * was caused by esp_wifi_disconnect() or esp_wifi_stop() */
                        /* desperatly tries to reconnect */
                        wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*)event_data;

                        // just lost connection
                        if (wifi_connected)
                        {
                                wifi_connected = false;
                                LOGFE(s_TAG, "lost wifi connection rc=%d", event->reason);
                                LOGFE(s_TAG, "trying to reconnect...");
                        }

                        // try to reconnect
                        esp_wifi_connect();
                default:
                }
                return;
        }
        if (event_base == IP_EVENT)
        {
                switch (event_id)
                {
                /* if esp_wifi_connect() returns ESP_OK and the station successfully
                 * connects to the target AP, the connection event will arise. Upon
                 * receiving this event, the event task starts the DHCP client and begins
                 * the DHCP process of getting the IP address. This event arises when the
                 * DHCP client successfully gets the IPV4 address from the DHCP server, or
                 * when the IPV4 address is changed. The event means that everything is
                 * ready and the application can begin its tasks (e.g., creating sockets).
                 */
                case IP_EVENT_STA_GOT_IP:

                        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;

                        wifi_connected = true;
                        LOGFI(s_TAG, "connected to Wi-Fi");

                        // update ip info.
                        ip_info = event->ip_info;
                        LOGFI(s_TAG, "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
                        break;
                }
                return;
        }
}

// wifi sta initialization
esp_err_t wifi_init_sta(void)
{
        esp_err_t ret;

        // initialize TCP/IP network interface
        ret = esp_netif_init();
        ESP_ERROR_CHECK(ret);

        // start running event loop on backgroud
        ret = esp_event_loop_create_default();
        ESP_ERROR_CHECK(ret);

        // initialize wifi with default sta config
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ret                    = esp_wifi_init(&cfg);
        ESP_ERROR_CHECK(ret);

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL,
                                                  &instance_any_id);
        ESP_ERROR_CHECK(ret);
        ret = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL,
                                                  &instance_got_ip);
        ESP_ERROR_CHECK(ret);

        wifi_config_t wifi_config = {
            .sta = {
                    .ssid     = CONFIG_WIFI_SSID,
                    .password = CONFIG_WIFI_PASSWORD,
                    }
        };

        ret = esp_wifi_set_mode(WIFI_MODE_STA);
        ESP_ERROR_CHECK(ret);
        ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
        ESP_ERROR_CHECK(ret);

        ret = esp_wifi_start();
        ESP_ERROR_CHECK(ret); // WIFI_EVENT_STA_START

        return ret;
}
