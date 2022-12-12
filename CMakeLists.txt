# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

add_definitions(-DLV_HOR_RES_MAX=320)
add_definitions(-DLV_VER_RES_MAX=480)
add_definitions(-DSPI_HOST_MAX=SPI3_HOST)
add_definitions(-DI2C_DEVICES_STRUCT_TM_CONVERSION)
add_definitions(-DGEL_SCHEDULER_MAX_ENTRIES=6)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(moschito)
