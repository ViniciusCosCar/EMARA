#include "M5Unified.h"

#include "esp_log.h"
#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace
{
constexpr size_t kLogCount = 5;
constexpr size_t kLogLength = 128;

char s_recent_logs[kLogCount][kLogLength] = {};
size_t s_next_log = 0;
portMUX_TYPE s_log_mux = portMUX_INITIALIZER_UNLOCKED;
vprintf_like_t s_original_vprintf = nullptr;

int capture_log_vprintf(const char* format, va_list args)
{
        char line[kLogLength];
        va_list output_args;
        va_copy(output_args, args);
        const int result = vsnprintf(line, sizeof(line), format, args);
        va_end(args);

        // Keep only the message itself; newlines are drawn by the display task.
        for (size_t i = 0; line[i] != '\0'; ++i)
        {
                if (line[i] == '\r' || line[i] == '\n') line[i] = ' ';
        }

        portENTER_CRITICAL(&s_log_mux);
        strlcpy(s_recent_logs[s_next_log], line, sizeof(s_recent_logs[s_next_log]));
        s_next_log = (s_next_log + 1) % kLogCount;
        portEXIT_CRITICAL(&s_log_mux);

        const int output_result = s_original_vprintf ? s_original_vprintf(format, output_args) : result;
        va_end(output_args);
        return output_result;
}

void copy_recent_logs(char (&logs)[kLogCount][kLogLength])
{
        portENTER_CRITICAL(&s_log_mux);
        for (size_t i = 0; i < kLogCount; ++i)
        {
                const size_t index = (s_next_log + i) % kLogCount;
                strlcpy(logs[i], s_recent_logs[index], kLogLength);
        }
        portEXIT_CRITICAL(&s_log_mux);
}

void display_task(void*)
{
        char logs[kLogCount][kLogLength];
        char dashboard_ip[24] = "waiting for IP";

        while (true)
        {
                esp_netif_t* ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
                esp_netif_ip_info_t ip_info = {};
                if (ap_netif && esp_netif_get_ip_info(ap_netif, &ip_info) == ESP_OK && ip_info.ip.addr != 0)
                {
                        snprintf(dashboard_ip, sizeof(dashboard_ip), IPSTR, IP2STR(&ip_info.ip));
                }

                copy_recent_logs(logs);
                M5.Display.fillScreen(TFT_BLACK);
                M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
                M5.Display.setTextSize(1);
                M5.Display.setCursor(4, 4);
                M5.Display.printf("EMARA  Battery: %d%%\n", M5.Power.getBatteryLevel());
                M5.Display.printf("Wi-Fi: %s\n", CONFIG_AP_WIFI_SSID);
                M5.Display.printf("Password: %s\n", CONFIG_AP_WIFI_PASSWORD);
                M5.Display.printf("Dashboard: http://%s\n", dashboard_ip);
                M5.Display.println("Recent logs:");

                for (size_t i = 0; i < kLogCount; ++i)
                {
                        if (logs[i][0] == '\0') continue;
                        // Bound each log line to the available display width.
                        if (strlen(logs[i]) > 52) logs[i][52] = '\0';
                        M5.Display.println(logs[i]);
                }

                vTaskDelay(pdMS_TO_TICKS(1000));
        }
}
} // namespace

extern "C" void m5_display_start(void)
{
        M5.begin();
        M5.Display.setRotation(1);
        M5.Display.setTextSize(1);
        s_original_vprintf = esp_log_set_vprintf(capture_log_vprintf);
        xTaskCreate(display_task, "m5_display", 4096, nullptr, 1, nullptr);
}
