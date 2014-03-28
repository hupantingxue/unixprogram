cmake_minimum_required(VERSION 2.8)
PROJECT(dirtycheck)

set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set(LIBSRC_LIST dirtycheck.cpp  dirtycheck.cpp)
set(BINSRC_LIST main.cpp)

add_library(dirtycheck ${LIBSRC_LIST})
add_executable(check ${BINSRC_LIST})
target_link_libraries(check dirtycheck)
