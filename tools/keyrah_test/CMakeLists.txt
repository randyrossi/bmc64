cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(keyrah_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(keyrah_test
    main.c
    usb_descriptors.c
)

target_include_directories(keyrah_test PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# bInterval (ms), set separately for keyboards vs gamepad-shaped interfaces -
target_compile_definitions(keyrah_test PRIVATE
    KEYRAH_KBD_POLL_INTERVAL_MS=1
    KEYRAH_PAD_POLL_INTERVAL_MS=1
)

target_link_libraries(keyrah_test PRIVATE
    pico_stdlib
    tinyusb_device
    tinyusb_board
)

# UART only - USB is fully consumed by our own composite HID+CDC descriptor,
# so pico_stdlib's own "stdio over USB CDC" must stay off or it will fight our
# hand-rolled CDC console (see console_task() in main.c).
pico_enable_stdio_uart(keyrah_test 1)
pico_enable_stdio_usb(keyrah_test 0)

pico_add_extra_outputs(keyrah_test)
