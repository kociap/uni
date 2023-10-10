cmake_minimum_required(VERSION 3.18)
include(FetchContent)

project(jftt)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FIND_COMPILE_FLAGS
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
  GIT_TAG 75f4c8811aaa7200351eeaaf234365363834dd58
)
FetchContent_MakeAvailable(anton_core)

add_executable(kmp "${CMAKE_CURRENT_SOURCE_DIR}/matcher.cpp")
target_compile_definitions(kmp PRIVATE USE_FA_MATCHER=0)
set_target_properties(kmp PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
target_compile_options(kmp PRIVATE ${FIND_COMPILE_FLAGS})
target_link_libraries(kmp PUBLIC anton_core)
add_custom_command(TARGET kmp POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:kmp> "${CMAKE_CURRENT_SOURCE_DIR}/kmp")

add_executable(fa "${CMAKE_CURRENT_SOURCE_DIR}/matcher.cpp")
target_compile_definitions(fa PRIVATE USE_FA_MATCHER=1)
set_target_properties(fa PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS OFF)
target_compile_options(fa PRIVATE ${FIND_COMPILE_FLAGS})
target_link_libraries(fa PUBLIC anton_core)
add_custom_command(TARGET fa POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:fa> "${CMAKE_CURRENT_SOURCE_DIR}/fa")
