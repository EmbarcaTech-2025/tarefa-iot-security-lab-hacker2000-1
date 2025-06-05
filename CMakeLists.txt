# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

project(iot_security_lab C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# --- Bibliotecas comuns para ambos os executáveis ---
set(COMMON_LIBS
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico_lwip
    pico_cyw43_driver
)

set(COMMON_SOURCES
    src/mqtt_comm.c
    src/wifi_conn.c
    src/xor_cipher.c
)

# --- Executável 1: PUBLISHER ---
add_executable(iot_publisher
    iot_security_lab.c
    ${COMMON_SOURCES}
)
# >>> CORREÇÃO: Adiciona o diretório do projeto à lista de includes do publisher
target_include_directories(iot_publisher PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(iot_publisher PRIVATE ${COMMON_LIBS})
pico_add_extra_outputs(iot_publisher)


# --- Executável 2: SUBSCRIBER ---
add_executable(iot_subscriber
    iot_security_lab_subscriber.c
    ${COMMON_SOURCES}
)
# >>> CORREÇÃO: Adiciona o diretório do projeto à lista de includes do subscriber
target_include_directories(iot_subscriber PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(iot_subscriber PRIVATE ${COMMON_LIBS})
pico_add_extra_outputs(iot_subscriber)


# --- Configurações de Stdio (Opcional, pode ser por executável) ---
# Habilita a saída USB para ambos
pico_enable_stdio_usb(iot_publisher 1)
pico_enable_stdio_usb(iot_subscriber 1)
pico_enable_stdio_uart(iot_publisher 0)
pico_enable_stdio_uart(iot_subscriber 0)