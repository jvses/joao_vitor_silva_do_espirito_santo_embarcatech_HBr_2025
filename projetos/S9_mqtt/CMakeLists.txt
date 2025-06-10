# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(subscriber C CXX ASM)#Aqui é definido o programa que será embarcado. Alterne entre publisher e subscriber

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(publisher app/publisher.c app/wifi_conn.c app/mqtt_comm.c)
pico_set_program_name(publisher "Publisher")
pico_set_program_version(publisher "0.1")
# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(publisher 0)
pico_enable_stdio_usb(publisher 1)
# Add the standard library to the build
target_link_libraries(publisher
        pico_stdlib
        pico_lwip_mqtt
        pico_lwip
        pico_cyw43_arch_lwip_threadsafe_background
        )
# Add the standard include files to the build
target_include_directories(publisher PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
)
# Add any user requested libraries
target_link_libraries(publisher 
        )
pico_add_extra_outputs(publisher)

###############
# Add executable. Default name is the project name, version 0.1
add_executable(subscriber app/subscriber.c app/wifi_conn.c app/mqtt_comm.c)
pico_set_program_name(subscriber "Subscriber")
pico_set_program_version(subscriber "0.1")
# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(subscriber 0)
pico_enable_stdio_usb(subscriber 1)
# Add the standard library to the build
target_link_libraries(subscriber
        pico_stdlib
        pico_lwip_mqtt
        pico_lwip
        pico_cyw43_arch_lwip_threadsafe_background
        )
# Add the standard include files to the build
target_include_directories(subscriber PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/include
)
# Add any user requested libraries
target_link_libraries(subscriber 
        )
pico_add_extra_outputs(subscriber)