#pragma once

namespace glang::ir {
  enum struct ALU_Opcode {
    add,
    sub,
    mul,
    div,
    mod,
    shl,
    shr,
    eq,
    neq,
    lt,
    gt,
    leq,
    geq,
  };

  [[nodiscard]] bool is_relational(ALU_Opcode op);
} // namespace glang::ir
