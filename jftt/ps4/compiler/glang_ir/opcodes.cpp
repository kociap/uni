#include <glang_ir/opcodes.hpp>

namespace glang::ir {
  bool is_relational(ALU_Opcode const op)
  {
    return op == ALU_Opcode::eq || op == ALU_Opcode::neq ||
           op == ALU_Opcode::gt || op == ALU_Opcode::lt ||
           op == ALU_Opcode::geq || op == ALU_Opcode::leq;
  }

} // namespace glang::ir
