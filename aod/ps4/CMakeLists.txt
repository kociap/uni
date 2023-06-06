cmake_minimum_required(VERSION 3.15)
include(FetchContent)

project(aod)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(edmondskarp
  "${CMAKE_CURRENT_SOURCE_DIR}/ex1_main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/graph.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/edmondskarp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/edmondskarp.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/options.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/options.cpp"
)
set_target_properties(edmondskarp PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_include_directories(edmondskarp PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(edmondskarp
  PUBLIC
  -Wall
  -Wextra
  -pedantic
  -fdiagnostics-color=always
  -ferror-limit=1

  -fno-rtti
  -fno-exceptions
  -fno-math-errno
  -fno-char8_t
)

add_executable(bipartite
  "${CMAKE_CURRENT_SOURCE_DIR}/ex2_main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/graph.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/edmondskarp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/edmondskarp.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/options.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/options.cpp"
)
set_target_properties(bipartite PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_include_directories(bipartite PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(bipartite
  PUBLIC
  -Wall
  -Wextra
  -pedantic
  -fdiagnostics-color=always
  -ferror-limit=1

  -fno-rtti
  -fno-exceptions
  -fno-math-errno
  -fno-char8_t
)
