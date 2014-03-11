cmake_minimum_required(VERSION 2.8)
PROJECT(md5)

set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set(LIBSRC_LIST md5.cpp)
set(BINSRC_LIST test_md5.cpp)

add_library(md5 ${LIBSRC_LIST})
add_executable(test_md5 ${BINSRC_LIST})
target_link_libraries(test_md5 md5)
