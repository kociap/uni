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
  GIT_TAG 75f4c8811aaa7200351eeaaf234365363834dd58
)
FetchContent_MakeAvailable(anton_core)

add_executable(entropy "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
set_target_properties(entropy PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(entropy PRIVATE ${KKD_COMPILE_FLAGS})
target_link_libraries(entropy PUBLIC anton_core)
add_custom_command(TARGET entropy POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:entropy> "${CMAKE_CURRENT_SOURCE_DIR}/entropy")
