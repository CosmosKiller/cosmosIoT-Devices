# Component CMake for lilFlowerPal 'src' component
# Collect all C/C++ sources in this directory and export needed include dirs

idf_component_register(SRCS "main.cpp" "pump_task.cpp" "bme680_task.cpp" "soil_moisture_task.cpp" "matter_task.cpp"
                       INCLUDE_DIRS "." "../tasks")

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")