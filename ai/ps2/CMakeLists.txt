cmake_minimum_required(VERSION 3.15)

project(tic_tac_toe)

add_executable(ttt_client
  "${CMAKE_CURRENT_SOURCE_DIR}/bot.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/bot.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/client.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/client.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/configuration.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/heuristic.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/heuristic.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
)
set_target_properties(ttt_client PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_include_directories(ttt_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(ttt_client
  PRIVATE
  -Wall
  -Wextra
  -pedantic
  -fdiagnostics-color=always

  -fno-rtti
  -fno-exceptions
  -fno-math-errno
  -fno-char8_t
)

add_executable(ttt_server
  "${CMAKE_CURRENT_SOURCE_DIR}/server.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/board.h"
)
set_target_properties(ttt_server PROPERTIES C_STANDARD 11 C_EXTENSIONS OFF)
target_include_directories(ttt_server PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(ttt_server
  PRIVATE
  -Wall
  -Wextra
  -pedantic
  -fdiagnostics-color=always
)
