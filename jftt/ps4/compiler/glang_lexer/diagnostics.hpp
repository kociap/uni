#pragma once

#include <glang_core/types.hpp>
#include <glang_diagnostics/error.hpp>

namespace glang {
  struct FE_Context;

  [[nodiscard]] Error
  err_lexer_unrecognised_token(FE_Context const& ctx,
                               anton::String_View source_path, i64 offset,
                               i64 line, i64 column);
} // namespace glang
