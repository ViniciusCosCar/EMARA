#ifndef APP_WIFI_H
#define APP_WIFI_H

#include "esp_err.h"
#include "esp_wifi.h"

// wifi state and current ip
extern bool wifi_connected;
extern esp_netif_ip_info_t ip_info;

// initialize WiFi as a station
esp_err_t wifi_init_sta(void);

#endif
