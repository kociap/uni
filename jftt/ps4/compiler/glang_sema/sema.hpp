#pragma once

#include <anton/expected.hpp>

#include <glang_ast/ast.hpp>
#include <glang_diagnostics/error.hpp>
#include <glang_frontend/fe_context.hpp>

namespace glang {
  [[nodiscard]] anton::Expected<void, Error> run_sema(FE_Context& ctx,
                                                      ast::Node_List nodes);
}
