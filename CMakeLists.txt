cmake_minimum_required(VERSION 3.30)
project(warpgate)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(external/rpclib)


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
target_include_directories(warpgate PUBLIC
        external/argparse/include
        external/rpclib/include
        external/nlohmann/single_include
        src)
target_link_libraries(warpgate PRIVATE rpc)
add_dependencies(warpgate tasklib)


add_library(tasklib SHARED
        src/tasklib/tasklib.cpp)
target_include_directories(tasklib PUBLIC src)