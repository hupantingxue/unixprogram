cmake_minimum_required(VERSION 2.8)
PROJECT(test)

set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set(SRC_LIST test01.cpp)
set(SRC_LIST2 test02.cpp)

add_executable(test01 ${SRC_LIST})
add_executable(test02 ${SRC_LIST2})
