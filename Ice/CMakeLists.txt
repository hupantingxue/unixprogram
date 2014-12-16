cmake_minimum_required(VERSION 2.8)
PROJECT(PrintDemo)

set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

include_directories("./")

set(SRC_LIST server.cpp Printer.cpp)
set(SRC_LIST_CLI client.cpp Printer.cpp)

add_executable(server ${SRC_LIST})
add_executable(client ${SRC_LIST_CLI})
target_link_libraries(server Ice IceUtil)
target_link_libraries(client Ice IceUtil)
