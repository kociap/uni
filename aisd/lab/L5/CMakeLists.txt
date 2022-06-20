cmake_minimum_required(VERSION 3.12)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/o")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/o")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/o")

project(L5 CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(heap "heap.cpp")
target_compile_definitions(heap PRIVATE HEAP=1)
add_executable(heapsort "heap.cpp")
target_compile_definitions(heapsort PRIVATE HEAPSORT=1)
add_executable(lcs "lcs.cpp")
add_executable(seqgen "seqgen.cpp")
add_executable(randgen "randgen.cpp")
