cmake_minimum_required(VERSION 3.15)

project(puzzle-solver CXX)

add_executable(solver
  "${CMAKE_CURRENT_SOURCE_DIR}/debug.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/heap.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/heuristic.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/heuristic.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/solver.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/solver.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
)
set_target_properties(solver PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_include_directories(solver PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(solver
  PRIVATE
  -Wall
  -Wextra
  -pedantic

  -fno-rtti
  -fno-exceptions
  -fno-math-errno
  -fno-char8_t
)

target_compile_definitions(solver PRIVATE ENABLE_DEBUG=${ENABLE_DEBUG})
