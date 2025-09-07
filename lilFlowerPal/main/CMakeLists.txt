# Component CMake for lilFlowerPal 'src' component
# Collect all C/C++ sources in this directory and export needed include dirs

file(GLOB SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

idf_component_register(SRCS ${SRC_FILES}
                       INCLUDE_DIRS "." "../tasks")