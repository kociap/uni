#pragma once

#include <anton/expected.hpp>

#include <glang_ast/ast.hpp>
#include <glang_diagnostics/error.hpp>
#include <glang_frontend/fe_context.hpp>
#include <glang_syntax/syntax.hpp>

namespace glang {
  // lower_syntax_to_ast
  //
  [[nodiscard]] anton::Expected<ast::Node_List, Error>
  lower_syntax_to_ast(FE_Context& ctx, anton::Slice<SNOT const> const syntax);
} // namespace glang
