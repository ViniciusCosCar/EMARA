idf_component_register(SRCS "app_wifi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_event log esp_wifi utils
                    )
