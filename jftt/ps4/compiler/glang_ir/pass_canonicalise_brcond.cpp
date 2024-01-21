#include <glang_ir/passes.hpp>

#include <glang_ir/passes.hpp>

#include <anton/flat_hash_set.hpp>

#include <glang_core/ilist.hpp>
#include <glang_ir/ir.hpp>
#include <glang_ir/opcodes.hpp>

namespace glang {
  static void process_block(ir::Context& ctx, ir::Block* const block)
  {
    ir::Instr* current = block->get_first();
    ir::Instr* const end = block->get_last();
    for(; current != end; current = current->next) {
      if(current->instr_kind != ir::Instr_Kind::alu) {
        continue;
      }

      ir::Instr_ALU* alu = static_cast<ir::Instr_ALU*>(current);
      if(!is_relational(alu->opcode)) {
        continue;
      }

      switch(alu->opcode) {
      case ir::ALU_Opcode::eq: {
        alu->opcode = ir::ALU_Opcode::neq;
        for(ir::Instr* const referrer: alu->referrers) {
          ANTON_ASSERT(referrer->instr_kind == ir::Instr_Kind::brcond,
                       "relational referrer is not conditional branch");
          auto const instr = static_cast<ir::Instr_brcond*>(referrer);
          anton::swap(instr->then_target, instr->else_target);
        }
      } break;

      case ir::ALU_Opcode::geq: {
        alu->opcode = ir::ALU_Opcode::lt;
        for(ir::Instr* const referrer: alu->referrers) {
          ANTON_ASSERT(referrer->instr_kind == ir::Instr_Kind::brcond,
                       "relational referrer is not conditional branch");
          auto const instr = static_cast<ir::Instr_brcond*>(referrer);
          anton::swap(instr->then_target, instr->else_target);
        }
      } break;

      case ir::ALU_Opcode::leq: {
        alu->opcode = ir::ALU_Opcode::gt;
        for(ir::Instr* const referrer: alu->referrers) {
          ANTON_ASSERT(referrer->instr_kind == ir::Instr_Kind::brcond,
                       "relational referrer is not conditional branch");
          auto const instr = static_cast<ir::Instr_brcond*>(referrer);
          anton::swap(instr->then_target, instr->else_target);
        }
      } break;

      case ir::ALU_Opcode::neq:
      case ir::ALU_Opcode::gt:
      case ir::ALU_Opcode::lt:
        break;

      default:
        ANTON_UNREACHABLE("unhandled opcode");
      }
    }
  }

  static void process_blocks(ir::Context& ctx, ir::Block* const block,
                             anton::Flat_Hash_Set<ir::Block*>& visited)
  {
    if(visited.find(block) != visited.end()) {
      return;
    }

    visited.emplace(block);
    process_block(ctx, block);
    ir::Instr* const terminator = block->get_last();
    ANTON_ASSERT(ir::is_branch(terminator), "terminator is not a branch");
    if(terminator->instr_kind == ir::Instr_Kind::branch) {
      auto const instr = static_cast<ir::Instr_branch*>(terminator);
      process_blocks(ctx, instr->target, visited);
    } else if(terminator->instr_kind == ir::Instr_Kind::brcond) {
      auto const instr = static_cast<ir::Instr_brcond*>(terminator);
      process_blocks(ctx, instr->then_target, visited);
      process_blocks(ctx, instr->else_target, visited);
    }
  }

  void canonicalise_brcond(ir::Context& ctx, ir::Function* const function)
  {
    anton::Flat_Hash_Set<ir::Block*> visited(ctx.allocator);
    process_blocks(ctx, function->entry_block, visited);
  }
} // namespace glang
