#pragma once

#include <glang_ir/ir.hpp>

namespace glang {
  void canonicalise_brcond(ir::Context& ctx, ir::Function* function);
  void lower_regs_to_mem(ir::Context& ctx, ir::Function* function);

  [[nodiscard]] bool opt_coalesce_loads(ir::Context& ctx,
                                        ir::Function* function);
} // namespace glang
