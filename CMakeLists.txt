cmake_minimum_required(VERSION 3.30)
project(warpgate)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/rpclib)

# ----- SERVER -----
add_executable(server
        src/server/server.cpp)
target_include_directories(server PUBLIC
        external/argparse/include
        external/rpclib/include
        src)
target_link_libraries(server PRIVATE rpc)
# ----- CLIENT -----
add_executable(client
        src/client/client.cpp
)
target_include_directories(client PUBLIC
        external/argparse/include
        external/rpclib/include
        src)
target_link_libraries(client PRIVATE rpc)
