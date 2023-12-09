cmake_minimum_required(VERSION 3.18)
include(FetchContent)

project(glangc CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

set(GLANGC_COMPILE_FLAGS
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
  # -Wunreachable-code-break

  -ftime-trace

  -Wno-reorder-init-list # Supress warnings about reordering in designated initializers
  -Wno-invalid-unevaluated-string # Fires in asserts causing spam
  -Wno-missing-field-initializers # TODO: Temporarily because of spam from glangc_diagnostics/diagnostics.hpp
)

set(ANTON_BUILD_DEBUG $<CONFIG:Debug>)
set(ANTON_ITERATOR_DEBUG $<CONFIG:Debug>)
set(ANTON_OPTIONAL_CHECK_VALUE $<CONFIG:Debug>)
set(ANTON_UNREACHABLE_ASSERTS ON)

# Add anton_core
FetchContent_Declare(
  anton_core
  GIT_REPOSITORY https://github.com/kociap/anton_core.git
  GIT_TAG dd3104f57d14d56570fc00221bd2f0976097176a
)
FetchContent_MakeAvailable(anton_core)

add_executable(glangc
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glangc.cpp"
)
set_target_properties(glangc PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)
target_compile_options(glangc PRIVATE ${GLANGC_COMPILE_FLAGS})
target_link_libraries(glangc PUBLIC anton_core)
target_include_directories(glangc PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/compiler")
target_sources(glangc
  PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_core/either.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_core/source_info.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_core/types.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_diagnostics/error.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_diagnostics/error.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_frontend/fe_context.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_lexer/diagnostics.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_lexer/diagnostics.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_lexer/lexer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_lexer/lexer.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_syntax/parser.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_syntax/parser.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_syntax/syntax.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler/glang_syntax/syntax.hpp"
)
