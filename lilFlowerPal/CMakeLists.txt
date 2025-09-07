cmake_minimum_required(VERSION 3.16)

# Make common/component folders visible to ESP-IDF so headers in .commonFiles are found
set(EXTRA_COMPONENT_DIRS "../.commonFiles/lib/cosmos_devices"
                         "../.commonFiles/lib/cosmos_power"
                         "../.commonFiles/lib/cosmos_sensor"
                         )

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lilFlowerPal)