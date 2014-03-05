cmake_minimum_required(VERSION 2.8)
PROJECT(signal)

set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set(SRC_LIST signal01.cpp)

add_executable(signal01 ${SRC_LIST})
