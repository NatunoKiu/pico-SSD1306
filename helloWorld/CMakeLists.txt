cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH /home/meee/pico-sdk)

include(pico_sdk_import.cmake)
project(helloWorld)
pico_sdk_init()


add_executable(helloWorld
	SSD1306Test.c
)


target_link_libraries(helloWorld pico_stdlib hardware_adc hardware_i2c)

pico_enable_stdio_usb(helloWorld 1)
pico_enable_stdio_uart(helloWorld 0)
pico_add_extra_outputs(helloWorld)