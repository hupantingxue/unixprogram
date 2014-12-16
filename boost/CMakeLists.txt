cmake_minimum_required(VERSION 2.8)
PROJECT(ptr)

set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set(BINSRC_LIST ptr.cpp)

add_executable(ptr ${BINSRC_LIST})
target_link_libraries(ptr boost)
