idf_component_register(SRCS "esp_idf_webserver.cpp"
                        INCLUDE_DIRS "."
                        REQUIRES app_wifi esp_http_server nvs_flash m5_display
                        EMBED_FILES
                                "dashboard/templates/index.html"
                                "dashboard/js/index.js"
                                "dashboard/css/index.css"
)
