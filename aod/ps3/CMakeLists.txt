cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(graphs)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_library(graphs
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/graph.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/dial.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/radix.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/dijkstra.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/utility.hpp"
)
set_target_properties(graphs PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_include_directories(graphs PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(graphs
  PUBLIC
  -Wall
  -Wextra
  -pedantic
  -fdiagnostics-color=always

  -fno-rtti
  -fno-exceptions
  -fno-math-errno
  -fno-char8_t
)

add_executable(dijkstra
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_link_libraries(dijkstra PRIVATE graphs)
set_target_properties(dijkstra PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_definitions(dijkstra PRIVATE ALGORITHM_DIJKSTRA=1)

add_executable(dial
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_link_libraries(dial PRIVATE graphs)
set_target_properties(dial PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_definitions(dial PRIVATE ALGORITHM_DIAL=1)

add_executable(radix
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)
target_link_libraries(radix PRIVATE graphs)
set_target_properties(radix PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_definitions(radix PRIVATE ALGORITHM_RADIX=1)
