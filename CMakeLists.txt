cmake_minimum_required(VERSION 3.8)

project(libdiscord VERSION 0.3.0 LANGUAGES C)


set(CMAKE_C_STANDARD 99)

include_directories(example_bots)
include_directories(lib)

set(LD_LIBS websockets ulfius yder orcania jansson config)

set(LD_SOURCE lib/libdiscord.c lib/libdiscord.h)
add_library(libdiscord SHARED ${LD_SOURCE})

set_target_properties(libdiscord PROPERTIES VERSION ${PROJECT_VERSION})
target_compile_options(libdiscord PRIVATE "-Wall")
target_compile_options(libdiscord PRIVATE "-Werror")

set(BOT1_SOURCE example_bots/example-bot-1.c)
add_executable(example-bot-1 ${BOT1_SOURCE})

target_link_libraries(example-bot-1 libdiscord)
