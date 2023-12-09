#pragma once

#include <anton/expected.hpp>

#include <glang_core/types.hpp>
#include <glang_diagnostics/error.hpp>
#include <glang_syntax/syntax.hpp>

namespace glang {
  struct Token;
  struct FE_Context;

  // parse_tokens
  // Builds syntax tree from tokenised source code.
  //
  // Parameters:
  // source_path - Name of the source. Must be address-stable and persist for at
  //               least as long as the syntax tree.
  //      tokens - The tokenised source code to be parsed.
  //
  [[nodiscard]] anton::Expected<anton::Array<SNOT>, Error>
  parse_tokens(FE_Context const& ctx, anton::String_View source_path,
               anton::Slice<Token const> source_code);
} // namespace glang
