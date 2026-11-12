
#include "M5Unified.h"
#include "app_wifi.h"

#include "esp_netif.h"
#include "esp_netif_ip_addr.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "inttypes.h"
#include "sdkconfig.h"

namespace UI
{
constexpr uint16_t BG      = 0x0841; // Dark navy
constexpr uint16_t PANEL   = 0x10A3;
constexpr uint16_t CYAN    = 0x07FF;
constexpr uint16_t BLUE    = 0x3DFF;
constexpr uint16_t GREEN   = 0x07E0;
constexpr uint16_t YELLOW  = 0xFFE0;
constexpr uint16_t ORANGE  = 0xFD20;
constexpr uint16_t RED     = 0xF800;
constexpr uint16_t WHITE   = 0xFFFF;
constexpr uint16_t GREY    = 0x9CF3;
constexpr uint16_t MAGENTA = 0xF81F;

void title(const char* text, int x, int y, uint16_t color)
{
        M5.Display.setTextColor(color, BG);
        M5.Display.setCursor(x, y);
        M5.Display.printf("%s", text);
}

void panel(int x, int y, int w, int h, uint16_t accent)
{
        M5.Display.fillRoundRect(x, y, w, h, 6, PANEL);
        M5.Display.fillRoundRect(x, y, 4, h, 2, accent);
}

void battery(int x, int y, int percentage)
{
        percentage = constrain(percentage, 0, 100);

        constexpr int w = 38;
        constexpr int h = 16;

        M5.Display.drawRoundRect(x, y, w, h, 3, WHITE);
        M5.Display.fillRect(x + w, y + 4, 3, 8, WHITE);

        int fw = ((w - 4) * percentage) / 100;

        uint16_t color;

        if (percentage > 70)
                color = GREEN;
        else if (percentage > 40)
                color = YELLOW;
        else if (percentage > 20)
                color = ORANGE;
        else
                color = RED;

        if (fw > 0) M5.Display.fillRect(x + 2, y + 2, fw, h - 4, color);

        M5.Display.setTextColor(color, BG);
        M5.Display.setCursor(x + w + 7, y + 4);
        M5.Display.printf("%d%%", percentage);
}
} // namespace UI

void display_task(void*)
{
        char dashboard_ip[24] = "Waiting for IP";

        while (true)
        {
                esp_netif_t* ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");

                esp_netif_ip_info_t ip_info = {};

                if (ap_netif && esp_netif_get_ip_info(ap_netif, &ip_info) == ESP_OK && ip_info.ip.addr != 0)
                {
                        snprintf(dashboard_ip, sizeof(dashboard_ip), IPSTR, IP2STR(&ip_info.ip));
                }

                const int battery_level = static_cast<int>(M5.Power.getBatteryLevel());

                M5.Display.fillScreen(UI::BG);

                M5.Display.setTextSize(1);

                // HEADER
                UI::title("DEVICE DASHBOARD", 6, 5, UI::CYAN);

                UI::battery(M5.Display.width() - 85, 4, battery_level);

                M5.Display.drawFastHLine(4, 20, M5.Display.width() - 8, UI::CYAN);

                // WIFI PANEL
                UI::panel(4, 27, M5.Display.width() - 8, 48, UI::MAGENTA);

                UI::title("WIRELESS ACCESS POINT", 12, 32, UI::MAGENTA);

                M5.Display.setTextColor(UI::GREY, UI::PANEL);
                M5.Display.setCursor(12, 47);
                M5.Display.printf("SSID: ");

                M5.Display.setTextColor(UI::WHITE, UI::PANEL);
                M5.Display.printf("%s", CONFIG_AP_WIFI_SSID);

                M5.Display.setCursor(12, 62);
                M5.Display.setTextColor(UI::GREY, UI::PANEL);
                M5.Display.printf("KEY: ");

                M5.Display.setTextColor(UI::YELLOW, UI::PANEL);
                M5.Display.printf("%s", CONFIG_AP_WIFI_PASSWORD);

                // NETWORK PANEL
                UI::panel(4, 81, M5.Display.width() - 8, 48, UI::BLUE);

                UI::title("WEB DASHBOARD", 12, 86, UI::BLUE);

                M5.Display.setTextColor(UI::GREY, UI::PANEL);
                M5.Display.setCursor(12, 103);
                M5.Display.printf("HTTP://");

                M5.Display.setTextColor(UI::CYAN, UI::PANEL);
                M5.Display.setCursor(12, 115);
                M5.Display.printf("%s", dashboard_ip);

                // GNSS PANEL
                UI::panel(4, 135, M5.Display.width() - 8, 38, UI::GREEN);

                UI::title("GNSS POSITION", 12, 140, UI::GREEN);

                M5.Display.setTextColor(UI::YELLOW, UI::PANEL);
                M5.Display.setCursor(12, 156);
                M5.Display.printf("LAT: ?   LNG: ?");

                // FOOTER
                M5.Display.drawFastHLine(4, M5.Display.height() - 12, M5.Display.width() - 8, UI::GREY);

                M5.Display.setTextColor(UI::GREY, UI::BG);
                M5.Display.setCursor(5, M5.Display.height() - 9);
                M5.Display.printf((wifi_is_connected) ? "SYSTEM ONLINE" : "SYSTEM OFFLINE");

                vTaskDelay(pdMS_TO_TICKS(1000));
        }
}

extern "C" void m5_display_start(void)
{
        M5.begin();

        M5.Display.setRotation(1);
        M5.Display.setTextSize(1);

        xTaskCreate(display_task, "m5_display", 4096, nullptr, 1, nullptr);
}
