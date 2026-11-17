#ifndef APP_WIFI_H
#define APP_WIFI_H

#include "esp_err.h"

// Initialize WiFi as APsta + internet router
esp_err_t wifi_init_apsta(void);

extern volatile bool wifi_is_connected;

#endif
