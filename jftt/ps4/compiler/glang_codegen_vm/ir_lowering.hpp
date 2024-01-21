#pragma once

#include <glang_codegen_vm/lir.hpp>
#include <glang_core/ilist.hpp>
#include <glang_ir/ir.hpp>

namespace glang {
  [[nodiscard]] IList<lir::Instr>
  lower_ir_to_lir(Allocator* allocator, IList<ir::Function> const& functions);
}
