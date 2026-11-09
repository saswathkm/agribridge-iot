cmake_minimum_required(VERSION 3.13)

# Include Pico SDK macros
include(pico_sdk_import.cmake)

project(agribridge_field_node C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(field_node
    main.c
)

# Pull in Raspberry Pi Pico hardware libraries
target_link_libraries(field_node
    pico_stdlib
    hardware_spi
    hardware_adc
    hardware_gpio
)

# Enable USB CDC serial stdio, disable UART stdio by default (can be toggled)
pico_enable_stdio_usb(field_node 1)
pico_enable_stdio_uart(field_node 1)

# Generate UF2, bin, hex output
pico_add_extra_outputs(field_node)
