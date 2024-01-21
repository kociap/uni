#pragma once

#include <glang_codegen_vm/lir.hpp>

namespace glang {
  void assign_addresses(IList<lir::Instr>& instructions);
}
