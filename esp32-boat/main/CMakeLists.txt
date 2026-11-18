idf_component_register(SRCS "esp32-boat.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES trajectory utils packet LoRa
)
