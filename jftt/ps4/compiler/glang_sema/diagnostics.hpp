#pragma once

#include <glang_ast/ast.hpp>
#include <glang_diagnostics/error.hpp>
#include <glang_frontend/fe_context.hpp>

namespace glang {
  [[nodiscard]] Error err_symbol_redefinition(FE_Context& ctx,
                                              ast::Node const* original,
                                              ast::Node const* redefined);
  [[nodiscard]] Error err_symbol_undefined(FE_Context& ctx,
                                           ast::Node const* use);
  [[nodiscard]] Error
  err_called_symbol_is_not_function(FE_Context& ctx, ast::Stmt_Call const* call,
                                    ast::Node const* symbol);
} // namespace glang
