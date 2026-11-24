idf_component_register(SRCS "system_hander.cpp"
                    INCLUDE_DIRS "include"
		    REQUIRES esp_event utils packet LoRa
                    )
