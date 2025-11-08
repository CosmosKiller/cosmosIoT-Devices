idf_component_register(SRCS "lil_ui_task.c" "encoder_task.c" "lvgl_task.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_lvgl_port esp_lcd driver)