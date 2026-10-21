#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include <stdio.h>

// ————————————————————————————————————————————————————————————————————————————
//  PROVISORY macros and wifi_event_handler()
// ————————————————————————————————————————————————————————————————————————————
/* this is ok for now, but please
        * move wifi logic to a component and enhance reconnection system
        * enhance critical_err_check() and move all these macros to somewhere else
        so it can be used by all components
*/

#define idle()                                                                                                         \
        while (true) vTaskDelay(pdMS_TO_TICKS(1000))

#define critical_err_check(tag, ret)                                                                                   \
        if (ret != ESP_OK)                                                                                             \
        {                                                                                                              \
                ESP_LOGE(tag, "CRITICAL ERROR: ABORTING...\nline: %d, func: %d", __LINE__, __PRETTY_FUNCTION__);       \
                /* abort();  */                                                                                        \
                idle();                                                                                                \
        }

#define ESP_LOGFN(tag, subject, fmt, ...)                                                                              \
        ESP_LOGI(tag, "func: %s, line: %d: %s -> " fmt, __func__, __LINE__, subject, ##__VA_ARGS__)

const char* TAG             = "main";
uint8_t RETRY_COUNT         = 0;
uint8_t MAXIMUM_RETRY_COUNT = 5;

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{

        if (event_base == WIFI_EVENT)
        {
                switch (event_id)
                {
                /* triggered by esp_wifi_start() returning ESP_OK and the current Wi-Fi mode configured to STA or
                 * STA/AP. Upon receiving this event, the event task will initialize the LwIP network interface (netif)
                 */
                case WIFI_EVENT_STA_START:
                        /* connect to configured AP */
                        ESP_LOGFN(TAG, "success", "wifi_driver started");
                        esp_wifi_connect();
                        break;

                /* If esp_wifi_connect() returns ESP_OK and the station successfully connects to the target AP, the
                 * connection event will arise. Upon receiving this event, the event task starts the DHCP client and
                 * begins the DHCP process of getting the IP address */
                case WIFI_EVENT_STA_CONNECTED:

                        ESP_LOGFN(
                            TAG, "success",
                            "connected to target AP. Starting DHCP client and the process of getting the IP address");
                        break;

                case WIFI_EVENT_STA_DISCONNECTED:
                        /* TODO: before trying to reconnect, verify if WIFI_EVENT_STA_DISCONNECT was caused by
                         * esp_wifi_disconnect() or esp_wifi_stop() */
                        /* desperatly tries to reconnect */
                        // wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*)event_data;

                        // try to reconnect
                        if (RETRY_COUNT < MAXIMUM_RETRY_COUNT)
                        {
                                esp_wifi_connect();
                                RETRY_COUNT++;
                                ESP_LOGFN(TAG, "error", "retry to connect to the AP (try nº %d of %d maximum)\nrc: %s",
                                          RETRY_COUNT, MAXIMUM_RETRY_COUNT);
                        }
                        else
                                ESP_LOGFN(TAG, "error", "connect to the AP failed");

                        break;

                default:
                }
                return;
        }
        if (event_base == IP_EVENT)
        {
                switch (event_id)
                {
                /* if esp_wifi_connect() returns ESP_OK and the station successfully connects to the target AP, the
                 * connection event will arise. Upon receiving this event, the event task starts the DHCP client and
                 * begins the DHCP process of getting the IP address. This event arises when the DHCP client
                 * successfully gets the IPV4 address from the DHCP server, or when the IPV4 address is changed. The
                 * event means that everything is ready and the application can begin its tasks (e.g., creating
                 * sockets). */
                case IP_EVENT_STA_GOT_IP:

                        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;

                        ESP_LOGFN(TAG, "success", "connected to Wi-Fi");
                        RETRY_COUNT = 0;

                        if (!event->ip_changed)
                        {
                                ESP_LOGFN(TAG, "STA", "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
                        }

                        break;
                }
                return;
        }
}
void app_wifi_init(void)
{
        esp_err_t ret;

        // initialize TCP/IP network interface
        ret = esp_netif_init();
        critical_err_check(TAG, ret);

        // start running event loop on backgroud
        ret = esp_event_loop_create_default();
        critical_err_check(TAG, ret);

        // register events
        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;
        ret = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL,
                                                  &instance_any_id);
        critical_err_check(TAG, ret);
        ret = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL,
                                                  &instance_got_ip);
        critical_err_check(TAG, ret);

        // wifi config
        esp_netif_create_default_wifi_sta();

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ret                    = esp_wifi_init(&cfg);
        critical_err_check(TAG, ret);

        wifi_config_t wifi_config = {
            .sta = {.ssid = "ssid", .password = "password"}
        };

        ret = esp_wifi_set_mode(WIFI_MODE_STA);
        critical_err_check(TAG, ret);
        ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
        critical_err_check(TAG, ret);

        ret = esp_wifi_start();
        critical_err_check(TAG, ret); // WIFI_EVENT_STA_START
}

void app_main(void)
{
        esp_err_t ret;
        // ————————————————————————————————————————————————————————————————————————————
        //  Inits
        // ————————————————————————————————————————————————————————————————————————————
        ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
                critical_err_check(TAG, nvs_flash_erase());
                critical_err_check(TAG, nvs_flash_init());
        }

        app_wifi_init();
        ESP_LOGFN(TAG, "success", "wifi completed");

        idle();
}
