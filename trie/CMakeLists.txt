cmake_minimum_required(VERSION 2.8)
PROJECT(trie)

set(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}/")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/lib/")

set(SRC_LIST trie.cpp)

include_directories("/usr/local/include/hat-trie")
link_directories("/usr/local/lib")

add_executable(trie ${SRC_LIST})
target_link_libraries(trie libhat-trie.a)
