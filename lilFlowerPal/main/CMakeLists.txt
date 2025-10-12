# Component CMake for lilFlowerPal 'src' component
# Collect all C/C++ sources in this directory and export needed include dirs

idf_component_register(SRCS "main.cpp" "pump_task.cpp" "bme680_task.cpp" "analog_sensor_task.cpp" "matter_task.cpp" "encoder_task.cpp" "lvgl_task.cpp" "lil_ui_task.cpp"
                       INCLUDE_DIRS "." "../tasks"
                       REQUIRES esp_matter cosmos_sensor bme680)


lvgl_port_create_c_image("qr_code/test_qr.png" "qr_code/" "ARGB8888" "NONE")
lvgl_port_add_images(${COMPONENT_LIB} "qr_code/")

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")