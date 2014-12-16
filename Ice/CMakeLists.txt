cmake_minimum_required(VERSION 2.8)
PROJECT(PrintDemo)

set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set(CPP_GEN_PATH "${CMAKE_SOURCE_DIR}/gen/cpp/")


include_directories("${PROJECT_ROOT_PATH}/gen/cpp/")

set(SRC_LIST server.cpp ${CPP_GEN_PATH}/Printer.cpp)
set(SRC_LIST_CLI client.cpp ${CPP_GEN_PATH}/Printer.cpp)

add_executable(server ${SRC_LIST})
add_executable(client ${SRC_LIST_CLI})
target_link_libraries(server Ice IceUtil)
target_link_libraries(client Ice IceUtil)
