idf_component_register( SRCS "esp_idf_webserver.cpp" "m5_display.cpp"
                        INCLUDE_DIRS "."
                        REQUIRES arduino M5Unified esp_netif
                        EMBED_FILES
                                "dashboard/templates/index.html"
                                "dashboard/js/index.js"
                                "dashboard/css/index.css"
)
