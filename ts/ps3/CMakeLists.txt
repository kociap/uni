cmake_minimum_required(VERSION 3.15)

project(frame)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(encoder
  "${CMAKE_CURRENT_SOURCE_DIR}/crc.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/crc.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/frame.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/frame.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/utility.hpp"
)
set_target_properties(encoder PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_include_directories(encoder PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(encoder PRIVATE FRAME_ENCODE=1)
target_compile_options(encoder
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

add_executable(decoder
  "${CMAKE_CURRENT_SOURCE_DIR}/crc.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/crc.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/frame.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/frame.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/utility.hpp"
)
set_target_properties(decoder PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_include_directories(decoder PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(decoder PRIVATE FRAME_DECODE=1)
target_compile_options(decoder
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

add_executable(csma
  "${CMAKE_CURRENT_SOURCE_DIR}/csma.cpp"
)
set_target_properties(csma PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_include_directories(csma PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(csma
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
