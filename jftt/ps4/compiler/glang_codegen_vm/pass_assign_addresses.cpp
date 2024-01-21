#include <glang_codegen_vm/passes.hpp>

#include <glang_codegen_vm/lir.hpp>

namespace glang {
  void assign_addresses(IList<lir::Instr>& instructions)
  {
    i64 ip = 0;
    for(lir::Instr* instr: instructions) {
      if(instr->incoming.size() == 0) {
        ip += 1;
        continue;
      }

      for(lir::Instr* incoming: instr->incoming) {
        switch(incoming->instr_kind) {
        case lir::Instr_Kind::jump: {
          auto const jump = static_cast<lir::Instr_jump*>(incoming);
          jump->jump_ip = ip;
        } break;

        case lir::Instr_Kind::jzero: {
          auto const jump = static_cast<lir::Instr_jzero*>(incoming);
          jump->jump_ip = ip;
        } break;

        case lir::Instr_Kind::jpos: {
          auto const jump = static_cast<lir::Instr_jpos*>(incoming);
          jump->jump_ip = ip;
        } break;

        default:
          ANTON_UNREACHABLE("unhandled instruction");
        }
      }

      ip += 1;
    }
  }
} // namespace glang
