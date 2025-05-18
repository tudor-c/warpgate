cmake_minimum_required(VERSION 3.30)
project(warpgate)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/rpclib)

# ----- TRACKER -----
add_executable(tracker
        src/tracker/main.cpp
        src/consts.h
        src/tracker/tracker.h
        src/tracker/tracker.cpp)
target_include_directories(tracker PUBLIC
        external/argparse/include
        external/rpclib/include
        src)
target_link_libraries(tracker PRIVATE rpc)

# ----- CLIENT -----
add_executable(client
        src/client/main.cpp
        src/consts.h
        src/client/client.h
        src/client/client.cpp
)
target_include_directories(client PUBLIC
        external/argparse/include
        external/rpclib/include
        src)
target_link_libraries(client PRIVATE rpc)

# ----- TASKLIB -----
add_library(tasklib SHARED
        src/tasklib/tasklib.cpp)
target_include_directories(tasklib PUBLIC src)