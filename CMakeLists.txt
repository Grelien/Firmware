# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

# Set the project name
set(PROJECT_NAME "motor_controller")

# Specify the custom partition table
set(PARTITION_TABLE_CSV "partitions.csv")

# Configure flash size to 2MB
set(ESPTOOLPY_FLASHSIZE "2MB")

# Include ESP-IDF build system
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Define the project
project(${PROJECT_NAME})

# Set C standard
set(CMAKE_C_STANDARD 99)

# Add compile definitions
add_compile_definitions(
    CONFIG_FREERTOS_HZ=1000
)