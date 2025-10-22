idf_component_register(SRCS cosmos_light.c
                       INCLUDE_DIRS "."
                       REQUIRES cosmos_devices
                       REQUIRES esp_driver_ledc)