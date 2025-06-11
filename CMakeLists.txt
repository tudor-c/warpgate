cmake_minimum_required(VERSION 3.30)
project(warpgate)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)
FetchContent_Declare(
        rpclib
        GIT_REPOSITORY https://github.com/rpclib/rpclib.git
        GIT_TAG v2.3.0
)
FetchContent_Declare(
        argparse
        GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.13.0
        GIT_SHALLOW ON
)

FetchContent_MakeAvailable(json rpclib argparse spdlog)

add_executable(warpgate
        src/main.cpp
        src/warpgate.cpp
        src/warpgate.h
        src/consts.h
        src/tracker/tracker.h
        src/tracker/tracker.cpp
        src/client/client.h
        src/client/client.cpp
        src/client/dynamic_library/library.h
        src/client/dynamic_library/library.cpp
        src/Task.cpp
        src/Task.h
        src/utils.cpp
        src/utils.h
        src/log.h
        src/types.h
)
target_compile_options(warpgate PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(warpgate PUBLIC src)

target_link_libraries(warpgate PRIVATE rpc)
target_link_libraries(warpgate PRIVATE argparse)
target_link_libraries(warpgate PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(warpgate PRIVATE spdlog::spdlog)

target_precompile_headers(warpgate PRIVATE
        <spdlog/spdlog.h>
        <spdlog/sinks/stdout_color_sinks.h>
        <argparse/argparse.hpp>
)

get_target_property(RPC_INCLUDE_DIRS rpc INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(warpgate SYSTEM PRIVATE ${RPC_INCLUDE_DIRS})

add_dependencies(warpgate tasklib)
add_library(tasklib SHARED
        src/tasklib/tasklib.cpp)
target_include_directories(tasklib PUBLIC src)