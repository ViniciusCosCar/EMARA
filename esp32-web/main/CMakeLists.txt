idf_component_register(SRCS "esp32-web.cpp"
                        INCLUDE_DIRS "."
			REQUIRES system_handler app_wifi esp_http_server nvs_flash m5_display packet trajectory LoRa
                        EMBED_FILES
                                "dashboard/templates/index.html"
                                "dashboard/js/index.js"
                                "dashboard/css/index.css"
)
