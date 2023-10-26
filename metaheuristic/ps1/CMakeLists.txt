cmake_minimum_required(VERSION 3.18)
include(FetchContent)

project(metaheuristic)

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

add_library(graphs
  "${CMAKE_CURRENT_SOURCE_DIR}/graph.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/graph.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/loader.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/loader.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/mst_to_tsp.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/mst_to_tsp.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/mst.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/mst.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/types.hpp"
)
set_target_properties(graphs PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_include_directories(graphs PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_options(graphs PRIVATE ${KKD_COMPILE_FLAGS})
target_link_libraries(graphs PUBLIC anton_core)

add_executable(mst
  "${CMAKE_CURRENT_SOURCE_DIR}/main_mst.cpp"
)
set_target_properties(mst PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(mst PRIVATE ${KKD_COMPILE_FLAGS})
target_link_libraries(mst PUBLIC graphs)
add_custom_command(TARGET mst POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mst> "${CMAKE_CURRENT_SOURCE_DIR}/mst")

add_executable(tsp
  "${CMAKE_CURRENT_SOURCE_DIR}/main_tsp.cpp"
)
set_target_properties(tsp PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(tsp PRIVATE ${KKD_COMPILE_FLAGS})
target_link_libraries(tsp PUBLIC graphs)
add_custom_command(TARGET tsp POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:tsp> "${CMAKE_CURRENT_SOURCE_DIR}/tsp")

add_executable(random
  "${CMAKE_CURRENT_SOURCE_DIR}/main_random.cpp"
)
set_target_properties(random PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(random PRIVATE ${KKD_COMPILE_FLAGS})
target_link_libraries(random PUBLIC graphs)
add_custom_command(TARGET random POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:random> "${CMAKE_CURRENT_SOURCE_DIR}/random")
