cmake_minimum_required(VERSION 3.0)
project(libdiscord VERSION 0.4.0 LANGUAGES C)

## Library info
set(PACKAGE libdiscord)
set(CPACK_PACKAGE_NAME libdiscord)

set(LD_VERSION_MAJOR 0)
set(LD_VERSION_MINOR 4)
set(LD_VERSION_PATCH .0)
set(LD_VERSION "${LD_VERSION_MAJOR}.${LD_VERSION_MINOR}${LD_VERSION_PATCH}")

## libdiscord options

## Discord-specific default options
set(LD_API_URL "\"https://discord.com/api\"")
set(LD_REST_API_VERSION "\"/v6\"")
set(LD_WS_API_VERSION 6)
#set(LD_GATEWAY_API_VERSION 3 "Only version 3 of the gateway API is supported")
set(LD_WEBSITE_URL "\"https://github.com/dxing97/libdiscord\"")

## Compilation
set(CMAKE_C_STANDARD 99)

## Include directories
include_directories(/usr/local/lib /usr/local/include)
include_directories(example_bots libdiscord cmake)

#set(LD_INCLUDE_DIRS ${GLIB_INCLUDE_DIRS})

set(LD_LIBS websockets jansson curl ${GLIB_LDFLAGS})

## Sources and headers
configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/libdiscord_config.h.in"
        "${PROJECT_BINARY_DIR}/libdiscord_config.h"
)

## Targets
## Library
add_library(discord SHARED libdiscord/libdiscord.c libdiscord/libdiscord.h libdiscord/codes.h libdiscord/log.h libdiscord/context.c libdiscord/context.h libdiscord/REST.c libdiscord/REST.h)

target_include_directories(discord PRIVATE ${LD_INCLUDE_DIRS})
target_link_libraries(discord ${LD_LIBS})
set_target_properties(discord PROPERTIES VERSION ${PROJECT_VERSION})

#target_compile_options(discord PRIVATE "-Wall")
#target_compile_options(discord PRIVATE "-Werror")
#target_compile_options(discord PRIVATE "-Wno-unknown-pragmas")
#target_compile_options(discord PRIVATE "-g")


## Example Bots
set(BOT_NULL_SOURCE
        example_bots/example-null.c
        ${PROJECT_BINARY_DIR}/libdiscord_config.h)
add_executable(example-bot-null ${BOT_NULL_SOURCE})
target_link_libraries(example-bot-null discord)

set(BOT_AYY_SOURCE
        example_bots/example-ayylmao.c
        ${PROJECT_BINARY_DIR}/libdiscord_config.h)
add_executable(example-bot-ayylmao ${BOT_AYY_SOURCE})
target_link_libraries(example-bot-ayylmao discord)

set(EXAMPLE_SIMPLEPOST_SOURCE
        ${PROJECT_BINARY_DIR}/libdiscord_config.h
        example_bots/example-simplepost.c)
add_executable(example-simplepost ${EXAMPLE_SIMPLEPOST_SOURCE})
target_link_libraries(example-simplepost discord)

## Tests
add_executable(test_init
        tests/test_init.c)
target_link_libraries(test_init discord)