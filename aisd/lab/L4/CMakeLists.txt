cmake_minimum_required(VERSION 3.12)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/o")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/o")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/o")

project(L4 CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(bst "bst.cpp")
add_executable(rbt "rbt.cpp")
add_executable(splay "splay.cpp")
# target_compile_definitions(dpqsort PRIVATE DPQSORT=1)
add_executable(lingen "lingen.cpp")
add_executable(rlingen "rlingen.cpp")
add_executable(randgen "randgen.cpp")
# add_executable(searcharg "searcharg.cpp")
