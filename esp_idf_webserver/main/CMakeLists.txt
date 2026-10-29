idf_component_register( SRCS "esp_idf_webserver.c"
                        INCLUDE_DIRS "."
                        EMBED_FILES 
                                "dashboard/templates/index.html" 
                                "dashboard/js/index.js"
                                "dashboard/css/index.css"
)
