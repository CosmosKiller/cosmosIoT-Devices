idf_component_register(SRCS cosmos_sensor.c
                       INCLUDE_DIRS "."
                       REQUIRES cosmos_devices
                       REQUIRES esp_adc)