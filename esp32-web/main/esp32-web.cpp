
#include "LoRa.hpp"
#include "freertos/FreeRTOS.h"

#include "Arduino.h"
#include "app_wifi.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_server.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "m5_display.h"
#include "nvs_flash.h"
#include "packet.hpp"
#include "system_handler.hpp"
#include "trajectory.hpp"
#include "utils.h"
#include <cstring>

const char* TAG = "webserver";

extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[] asm("_binary_index_html_end");
extern const char index_js_start[] asm("_binary_index_js_start");
extern const char index_js_end[] asm("_binary_index_js_end");
extern const char index_css_start[] asm("_binary_index_css_start");
extern const char index_css_end[] asm("_binary_index_css_end");

// —————————————————————————————————————————————————————————————————————————
//  webserver and request handlers
// —————————————————————————————————————————————————————————————————————————
// —— GET
// index
esp_err_t index_html_get_handler(httpd_req_t* req)
{
        httpd_resp_set_type(req, "text/html");
        return httpd_resp_send(req, index_html_start, index_html_end - index_html_start);
}
esp_err_t index_js_get_handler(httpd_req_t* req)
{
        httpd_resp_set_type(req, "application/javascript");
        return httpd_resp_send(req, index_js_start, index_js_end - index_js_start);
}
esp_err_t index_css_get_handler(httpd_req_t* req)
{
        httpd_resp_set_type(req, "text/css");
        return httpd_resp_send(req, index_css_start, index_css_end - index_css_start);
}

// —— POST
// trajectory
esp_err_t trajectory_post_handler(httpd_req_t* req)
{
        char buf[128];
        int ret, remaining = req->content_len;

        httpd_resp_set_type(req, "text/plain");

        LOGFI(TAG, "============ TRAJECTORY RECEIVED ============");
        while (remaining > 0)
        {
                // read data for the request
                if ((ret = httpd_req_recv(req, buf, (remaining < sizeof buf) ? (remaining) : (sizeof buf))) <= 0)
                {
                        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
                        {
                                // retry receiving if timeout occoured
                                continue;
                        }
                        httpd_resp_sendstr(req, "Trajectory reception failed");
                        return ESP_FAIL;
                }
                remaining -= ret;

                // log data received
                LOGFI(TAG, "%.*s", ret, buf);
        }
        LOGFI(TAG, "=============================================");

        // end response
        httpd_resp_sendstr(req, "Trajectory received successfully");

        return ESP_OK;
}

// —— uri
// index
const httpd_uri_t index_html_get_uri = {
    .uri = "/", .method = HTTP_GET, .handler = index_html_get_handler, .user_ctx = NULL};
const httpd_uri_t index_js_get_uri = {
    .uri = "/dashboard/js/index.js", .method = HTTP_GET, .handler = index_js_get_handler, .user_ctx = NULL};
const httpd_uri_t index_css_get_uri = {
    .uri = "/dashboard/css/index.css", .method = HTTP_GET, .handler = index_css_get_handler, .user_ctx = NULL};
// trajectory
const httpd_uri_t trajectory_post_uri = {
    .uri = "/trajectory", .method = HTTP_POST, .handler = trajectory_post_handler, .user_ctx = NULL};

// —— webserver
httpd_handle_t start_webserver()
{

        httpd_handle_t server = NULL;
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
        esp_err_t ret;

        ret = httpd_start(&server, &config);
        if (ret == ESP_OK)
        {
                ESP_LOGI(TAG, "Server started successfully, registering URI handlers...");
                // index
                httpd_register_uri_handler(server, &index_html_get_uri);
                httpd_register_uri_handler(server, &index_js_get_uri);
                httpd_register_uri_handler(server, &index_css_get_uri);
                // trajectory
                httpd_register_uri_handler(server, &trajectory_post_uri);
        }
        else
        {
                ESP_LOGI(TAG, "Failed to start the server");
        }

        return server;
}

// —————————————————————————————————————————————————————————————————————————
//  LoRa
// —————————————————————————————————————————————————————————————————————————
// TODO: replace pseudo: notifyFromISR() with xTaskNotifyFromISR()
void callback_on_pk_reception(int datasize)
{
        if (datasize > MAX_PACKET_SIZE || datasize < 0)
        {
                xTaskNotifyFromISR(system_handle, LORA_PACKET_RECEIVED_BUT_ERROR | FAILURE_RC_OVERFLOW,
                                   eSetValueWithOverwrite, NULL);
                return;
        }

        for (size_t i = 0; i < datasize; i++)
        {
                int intres = LoRa_read_intbyte();
                if (intres < 0)
                {
                        xTaskNotifyFromISR(system_handle, LORA_PACKET_RECEIVED_BUT_ERROR | FAILURE_RC_UNKNOWN,
                                           eSetValueWithOverwrite, NULL);
                        return;
                };
                pk_data[i] = static_cast<uint8_t>(intres);
        }

        xTaskNotifyFromISR(system_handle, LORA_PACKET_RECEIVED, eSetValueWithOverwrite, NULL);
        return;
}

// —————————————————————————————————————————————————————————————————————————
//  main application
// —————————————————————————————————————————————————————————————————————————
extern "C" void app_main(void)
{
        esp_err_t ret;

        initArduino();
        m5_display_start();

        // nvs_flash
        ret = nvs_flash_init();

        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ESP_ERROR_CHECK(nvs_flash_init());
        }
        // wifi
        ESP_ERROR_CHECK(wifi_init_apsta());

        // webserver
        httpd_handle_t server = start_webserver();
        if (!server) ESP_ERROR_CHECK(ESP_FAIL);

        // initalize LoRa driver
        LoRa_driver_init();
        LoRa_on_receive(callback_on_pk_reception);

        while (true)
        {
        }
}
