cmake_minimum_required(VERSION 3.30)
project(warpgate)

set(CMAKE_CXX_STANDARD 20)

#find_package(Crow)

# ----- SERVER -----
add_executable(server
        src/server/server.cpp)
#target_link_libraries(server PUBLIC Crow::Crow)
target_include_directories(server PUBLIC
        external/cpp-httplib
        external/Crow/include
        ${CMAKE_CURRENT_SOURCE_R}/src/)
target_precompile_headers(server PUBLIC [["crow.h"]])

# ----- CLIENT -----
add_executable(client
        src/client/client.cpp)
#target_link_libraries(client PUBLIC Crow::Crow)
target_include_directories(client PUBLIC
        external/cpp-httplib
        external/Crow/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/)
target_precompile_headers(client REUSE_FROM server)

