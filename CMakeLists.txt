cmake_minimum_required(VERSION 3.30)
project(warpgate)

set(CMAKE_CXX_STANDARD 20)

# ----- SERVER -----
add_executable(server
        src/server/server.cpp)
target_include_directories(server PUBLIC
        external/cpp-httplib
        external/Crow/include
        src)
target_precompile_headers(server PUBLIC [["crow.h"]])

# ----- CLIENT -----
add_executable(client
        src/client/client.cpp)
target_include_directories(client PUBLIC
        external/cpp-httplib
        external/Crow/include
        src)
target_precompile_headers(client REUSE_FROM server)

