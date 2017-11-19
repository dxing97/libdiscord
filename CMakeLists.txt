cmake_minimum_required(VERSION 3.8)
project(libdiscord VERSION 0.3.0 LANGUAGES C)

set(LD_VERSION_MAJOR 0)
set(LD_VERSION_MINOR .3)
set(LD_VERSION_PATCH )
set(LD_VERSION "${LD_VERSION_MAJOR}${LD_VERSION_MINOR}${LD_VERSION_PATCH}")

## Discord connection options
set(LD_API_URL "https://discordapp.com/api")
set(LD_REST_API_VERSION 6)
#set(LD_GATEWAY_API_VERSION 3 "Only version 3 of the gateway API is supported")

set(CMAKE_C_STANDARD 99)

set(PACKAGE libdiscord)
set(CPACK_PACKAGE_NAME )

include_directories(example_bots lib cmake)

configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/libdiscord_config.h.in"
        "${PROJECT_BINARY_DIR}/libdiscord_config.h"
)

include_directories("${PROJECT_BINARY_DIR}")

set(LD_LIBS websockets ulfius yder orcania jansson config)

set(LD_SOURCE lib/libdiscord.c lib/libdiscord.h lib/log.c lib/log.h)
add_library(libdiscord SHARED ${LD_SOURCE})

set_target_properties(libdiscord PROPERTIES VERSION ${PROJECT_VERSION})
target_compile_options(libdiscord PRIVATE "-Wall")
target_compile_options(libdiscord PRIVATE "-Werror")

set(BOT1_SOURCE example_bots/example-ayylmao.c cmake/libdiscord_config.h.in)
add_executable(example-bot-1 ${BOT1_SOURCE})

target_link_libraries(example-bot-1 libdiscord)
