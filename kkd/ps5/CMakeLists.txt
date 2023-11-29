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
  GIT_TAG 6b215b9f78a322fb806c983e1ba57c55c383f7de
)
FetchContent_MakeAvailable(anton_core)

add_executable(quantise
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/tga.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/tga.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
)
set_target_properties(quantise PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(quantise PRIVATE ${KKD_COMPILE_FLAGS})
target_include_directories(quantise PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(quantise PUBLIC anton_core)
