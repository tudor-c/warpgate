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
FetchContent_MakeAvailable(json rpclib argparse)

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
)
target_compile_options(warpgate PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(warpgate PUBLIC src)

target_link_libraries(warpgate PRIVATE rpc)
target_link_libraries(warpgate PRIVATE argparse)
target_link_libraries(warpgate PRIVATE nlohmann_json::nlohmann_json)

set_target_properties(rpc PROPERTIES COMPILE_OPTIONS "-w")

add_dependencies(warpgate tasklib)
add_library(tasklib SHARED
        src/tasklib/tasklib.cpp)
target_include_directories(tasklib PUBLIC src)