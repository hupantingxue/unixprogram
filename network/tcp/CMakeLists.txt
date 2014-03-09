cmake_minimum_required(VERSION 2.8)
PROJECT(tcp)

set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set(CLISRC_LIST client.cpp)
set(SVRSRC_LIST server.cpp)

add_executable(client ${CLISRC_LIST})
add_executable(server ${SVRSRC_LIST})
