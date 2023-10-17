cmake_minimum_required(VERSION 3.18)
include(FetchContent)

project(kkd)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(KKD_COMPILE_FLAGS
  # -march=native
  -fno-math-errno
  -fno-char8_t # Yea, just no.
  -fno-rtti
  -fno-exceptions
  -Wall -Wextra -pedantic
  -ferror-limit=1

  -Werror=return-type
  -Werror=uninitialized

  -Wnon-virtual-dtor
  -Wnewline-eof

  -Wno-reorder-init-list # Supress warnings about reordering in designated initializers
)

# Add anton_core
FetchContent_Declare(
  anton_core
  GIT_REPOSITORY https://github.com/kociap/anton_core.git
  GIT_TAG cfe4f94af1283969e48ffb401d595dc924d138b7
)
FetchContent_MakeAvailable(anton_core)

add_library(compression
  "${CMAKE_CURRENT_SOURCE_DIR}/AAC.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/AAC.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/bitio.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
)
set_target_properties(compression PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(compression PRIVATE ${KKD_COMPILE_FLAGS})
target_include_directories(compression PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(compression PUBLIC anton_core)

add_executable(encoder
  "${CMAKE_CURRENT_SOURCE_DIR}/encoder.cpp"
)
set_target_properties(encoder PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(encoder PRIVATE ${KKD_COMPILE_FLAGS})
target_link_libraries(encoder PRIVATE compression)
add_custom_command(TARGET encoder POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:encoder> "${CMAKE_CURRENT_SOURCE_DIR}/encoder")

add_executable(decoder
  "${CMAKE_CURRENT_SOURCE_DIR}/decoder.cpp"
)
set_target_properties(decoder PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(decoder PRIVATE ${KKD_COMPILE_FLAGS})
target_link_libraries(decoder PUBLIC compression)
add_custom_command(TARGET decoder POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:decoder> "${CMAKE_CURRENT_SOURCE_DIR}/decoder")
