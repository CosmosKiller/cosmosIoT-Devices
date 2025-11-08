# Component CMake for lilFlowerPal 'src' component
# Collect all C/C++ sources in this directory and export needed include dirs

idf_component_register(SRCS "main.cpp" "pir_sensor_task.cpp" "matter_task.cpp" #"http_stream_task.c" "http_server_task.c" "cam_task.c" 
                       INCLUDE_DIRS "." "../tasks"
                       REQUIRES esp_matter)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")

# Ensure the PSRAM cache fix is applied for this component
target_compile_options(${COMPONENT_LIB} PRIVATE -mfix-esp32-psram-cache-issue)