#pragma once

#include <glang_ast/ast.hpp>
#include <glang_ir/context.hpp>
#include <glang_ir/ir.hpp>

namespace glang {
  [[nodiscard]] IList<ir::Function> lower_ast_to_ir(ir::Context& ctx,
                                                    ast::Node_List nodes);
}
