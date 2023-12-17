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
  GIT_TAG dd3104f57d14d56570fc00221bd2f0976097176a
)
FetchContent_MakeAvailable(anton_core)

add_executable(encoder
  "${CMAKE_CURRENT_SOURCE_DIR}/encoder.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
)
set_target_properties(encoder PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(encoder PRIVATE ${KKD_COMPILE_FLAGS})
target_include_directories(encoder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(encoder PUBLIC anton_core)

add_executable(decoder
  "${CMAKE_CURRENT_SOURCE_DIR}/decoder.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
)
set_target_properties(decoder PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(decoder PRIVATE ${KKD_COMPILE_FLAGS})
target_include_directories(decoder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(decoder PUBLIC anton_core)

add_executable(noise
  "${CMAKE_CURRENT_SOURCE_DIR}/noise.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
)
set_target_properties(noise PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(noise PRIVATE ${KKD_COMPILE_FLAGS})
target_include_directories(noise PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(noise PUBLIC anton_core)

add_executable(verifier
  "${CMAKE_CURRENT_SOURCE_DIR}/verifier.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
)
set_target_properties(verifier PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(verifier PRIVATE ${KKD_COMPILE_FLAGS})
target_include_directories(verifier PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(verifier PUBLIC anton_core)
