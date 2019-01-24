cmake_minimum_required(VERSION 3.0)
project(libdiscord VERSION 0.3.0 LANGUAGES C)

set(LD_VERSION_MAJOR 0)
set(LD_VERSION_MINOR .3)
set(LD_VERSION_PATCH )
set(LD_VERSION "${LD_VERSION_MAJOR}${LD_VERSION_MINOR}${LD_VERSION_PATCH}")

## Discord connection options
set(LD_API_URL "\"https://discordapp.com/api\"")
set(LD_REST_API_VERSION "\"/v6\"")
set(LD_WS_API_VERSION 6)
#set(LD_GATEWAY_API_VERSION 3 "Only version 3 of the gateway API is supported")
set(LD_GITHUB_URL "\"https://github.com/dxing97/libdiscord\"") # do we need this?

set(CMAKE_C_STANDARD 99)

IF (CYGWIN)
    SET (CMAKE_FIND_LIBRARY_PREFIXES "")
    SET (CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".LIB" ".dll" ".DLL")
ELSE()
    SET (CMAKE_FIND_LIBRARY_PREFIXES "lib")
    SET (CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a")
ENDIF (CYGWIN)

set(PACKAGE libdiscord)
set(CPACK_PACKAGE_NAME libdiscord)

include_directories(/usr/local/lib /usr/local/include)
include_directories(example_bots lib cmake)

#homebrew on macOS
include_directories(/usr/local/opt/curl/include)
include_directories(/usr/local/opt/openssl/include)

configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/libdiscord_config.h.in"
        "${PROJECT_BINARY_DIR}/libdiscord_config.h"
)

include_directories("${PROJECT_BINARY_DIR}")
# If libwebsockets was compiled and installed in Cygwin:
include_directories(/usr/local/lib/include)

set(LD_LIBS websockets jansson curl crypto)

set(LD_SOURCE lib/libdiscord.c lib/libdiscord.h lib/REST.h lib/REST.c lib/log.c lib/log.h lib/json.c lib/json.h lib/utils.c lib/utils.h)
add_library(discord SHARED ${LD_SOURCE})

target_link_libraries(discord ${LD_LIBS})
set_target_properties(discord PROPERTIES VERSION ${PROJECT_VERSION})
target_compile_options(discord PRIVATE "-Wall")
target_compile_options(discord PRIVATE "-Werror")
target_compile_options(discord PRIVATE "-Wno-unknown-pragmas")


set(BOT_AYY_SOURCE example_bots/example-ayylmao.c cmake/libdiscord_config.h.in)
add_executable(example-bot-ayylmao ${BOT_AYY_SOURCE})
target_link_libraries(example-bot-ayylmao discord)

set(BOT_PING_SOURCE example_bots/example-ping.c cmake/libdiscord_config.h.in)
add_executable(example-bot-ping ${BOT_PING_SOURCE})
target_link_libraries(example-bot-ping discord)

set(BOT_COUNTER_SOURCE example_bots/example-counter.c cmake/libdiscord_config.h.in)
add_executable(example-bot-counter ${BOT_COUNTER_SOURCE})
target_link_libraries(example-bot-counter discord)

set(EXAMPLE_TESTING_SOURCE cmake/libdiscord_config.h.in example_bots/example-testing.c)
add_executable(example-testing ${EXAMPLE_TESTING_SOURCE})
target_link_libraries(example-testing discord)

set(BOT_HASH_SOURCE cmake/libdiscord_config.h.in example_bots/example-hash.c)
add_executable(example-bot-hash ${BOT_HASH_SOURCE})
target_link_libraries(example-bot-hash discord)