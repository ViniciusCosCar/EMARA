#include "freertos/FreeRTOS.h"

#include "app_wifi.h"
#include "esp_err.h"
#include "esp_http_server.h"
#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include "utils.h"

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
// TODO: send received trajectory to STM32WL through UART
// TODO: add trajectory verification before forwarding
esp_err_t trajectory_post_handler(httpd_req_t* req)
{
        char buf[64];
        int ret, remaining = req->content_len;

        httpd_resp_set_type(req, "text/plain");

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

                // Exemple for the TODO:
                // if(uart_send_chunk(buf, ret) < 0) // failure
                // {...}
                remaining -= ret;

                // log data received
                LOGFI(TAG, "—————————— RECEIVED TRAJECTORY ——————————");
                LOGFI(TAG, "%.*s", ret, buf);
                LOGFI(TAG, "———————————————————————————————————");
        }
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
                ESP_LOGI(TAG, "Server started successfully, registering URI handlers...");
        else
                ESP_LOGI(TAG, "Failed to start the server");

        // index
        httpd_register_uri_handler(server, &index_html_get_uri);
        httpd_register_uri_handler(server, &index_js_get_uri);
        httpd_register_uri_handler(server, &index_css_get_uri);
        // trajectory
        httpd_register_uri_handler(server, &trajectory_post_uri);

        return server;
}

// —————————————————————————————————————————————————————————————————————————
//  main application
// —————————————————————————————————————————————————————————————————————————
void app_main(void)
{
        esp_err_t ret;

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

        while (true) vTaskDelay(pdMS_TO_TICKS(500));
}
