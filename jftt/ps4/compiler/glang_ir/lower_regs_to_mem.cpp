#include <glang_ir/passes.hpp>

#include <anton/array.hpp>
#include <anton/flat_hash_set.hpp>

#include <glang_core/ilist.hpp>
#include <glang_ir/ir.hpp>
#include <glang_ir/opcodes.hpp>

namespace glang {

#define REPLACE(target)                                       \
  if(target == original) {                                    \
    ir::Instr* const load =                                   \
      ir::make_instr_load(ctx, instr->source_info, variable); \
    target = load;                                            \
    load->add_referrer(instr);                                \
    fills.insert_back(load);                                  \
  }

  static void replace_uses_with_load_var(ir::Context& ctx,
                                         ir::Instr* const user,
                                         ir::Instr* const original,
                                         ir::Instr_variable* const variable,
                                         IList<ir::Instr>& fills)
  {
    original->remove_referrer(user);
    switch(user->instr_kind) {
    case ir::Instr_Kind::getelementptr: {
      auto const instr = static_cast<ir::Instr_getelementptr*>(user);
      REPLACE(instr->offset);
    } break;

    case ir::Instr_Kind::getelementptr_indirect: {
      auto const instr = static_cast<ir::Instr_getelementptr_indirect*>(user);
      REPLACE(instr->offset);
    } break;

    case ir::Instr_Kind::store: {
      auto const instr = static_cast<ir::Instr_store*>(user);
      REPLACE(instr->dst);
      REPLACE(instr->src);
    } break;

    case ir::Instr_Kind::store_indirect: {
      auto const instr = static_cast<ir::Instr_store_indirect*>(user);
      REPLACE(instr->dst);
      REPLACE(instr->src);
    } break;

    case ir::Instr_Kind::load: {
      auto const instr = static_cast<ir::Instr_load*>(user);
      REPLACE(instr->src);
    } break;

    case ir::Instr_Kind::load_indirect: {
      auto const instr = static_cast<ir::Instr_load_indirect*>(user);
      REPLACE(instr->src);
    } break;

    case ir::Instr_Kind::load_rhs: {
      auto const instr = static_cast<ir::Instr_load_rhs*>(user);
      REPLACE(instr->src);
    } break;

    case ir::Instr_Kind::alu: {
      auto const instr = static_cast<ir::Instr_ALU*>(user);
      REPLACE(instr->rhs);
      REPLACE(instr->lhs);
    } break;

    case ir::Instr_Kind::brcond: {
      auto const instr = static_cast<ir::Instr_brcond*>(user);
      REPLACE(instr->condition);
    } break;

    case ir::Instr_Kind::call: {
      auto const instr = static_cast<ir::Instr_call*>(user);
      for(ir::Instr*& arg: instr->arguments) {
        REPLACE(arg);
      }
    } break;

    case ir::Instr_Kind::write: {
      auto const instr = static_cast<ir::Instr_write*>(user);
      REPLACE(instr->src);
    } break;

    case ir::Instr_Kind::read:
    case ir::Instr_Kind::ret:
    case ir::Instr_Kind::end:
    case ir::Instr_Kind::constant:
    case ir::Instr_Kind::variable:
    case ir::Instr_Kind::branch:
      // Does not refer to any other instructions.
      break;
    }
  }

  static void replace_uses_with_load_var(ir::Context& ctx,
                                         ir::Instr* instruction,
                                         ir::Instr_variable* variable,
                                         IList<ir::Instr>& fills)
  {
    for(ir::Instr* const referrer: instruction->referrers) {
      replace_uses_with_load_var(ctx, referrer, instruction, variable, fills);
    }
  }

  void reg2mem(ir::Context& ctx, ir::Function* const function,
               ir::Instr* const instr, IList<ir::Instr>& fills)
  {
    ir::Instr_variable* const local = ir::make_instr_variable(
      ctx, instr->source_info, ir::Variable_Kind::local, "spill");
    function->add_local(local);
    replace_uses_with_load_var(ctx, instr, local, fills);
    // Create store after replacing all uses. Otherwise it will replace the use
    // in the store as well.
    ir::Instr* const store =
      ir::make_instr_store(ctx, instr->source_info, local, instr);
    ilist_insert_after(instr, store);
  }

  static void process_block(ir::Context& ctx, ir::Block* const block,
                            IList<ir::Instr>& fills)
  {
    ir::Function* const function = block->function;
    ir::Instr* current = block->get_first();
    ir::Instr* const end = block->get_last();
    for(; current != end; current = current->next) {
      switch(current->instr_kind) {
      case ir::Instr_Kind::alu:
      case ir::Instr_Kind::constant:
      case ir::Instr_Kind::read:
      case ir::Instr_Kind::load:
      case ir::Instr_Kind::load_indirect:
      case ir::Instr_Kind::load_rhs:
      case ir::Instr_Kind::getelementptr:
      case ir::Instr_Kind::getelementptr_indirect:
        reg2mem(ctx, function, current, fills);

      case ir::Instr_Kind::store:
      case ir::Instr_Kind::store_indirect:
      case ir::Instr_Kind::write:
      case ir::Instr_Kind::end:
      case ir::Instr_Kind::ret:
      case ir::Instr_Kind::call:
      case ir::Instr_Kind::branch:
      case ir::Instr_Kind::brcond:
        break;

      case ir::Instr_Kind::variable:
        ANTON_UNREACHABLE("unhandled instruction");
      }
    }
  }

  static void process_blocks(ir::Context& ctx, ir::Block* const block,
                             anton::Flat_Hash_Set<ir::Block*>& visited,
                             IList<ir::Instr>& fills)
  {
    if(visited.find(block) != visited.end()) {
      return;
    }

    visited.emplace(block);
    process_block(ctx, block, fills);
    ir::Instr* const terminator = block->get_last();
    ANTON_ASSERT(ir::is_branch(terminator), "terminator is not a branch");
    if(terminator->instr_kind == ir::Instr_Kind::branch) {
      auto const instr = static_cast<ir::Instr_branch*>(terminator);
      process_blocks(ctx, instr->target, visited, fills);
    } else if(terminator->instr_kind == ir::Instr_Kind::brcond) {
      auto const instr = static_cast<ir::Instr_brcond*>(terminator);
      process_blocks(ctx, instr->then_target, visited, fills);
      process_blocks(ctx, instr->else_target, visited, fills);
    }
  }

  void lower_regs_to_mem(ir::Context& ctx, ir::Function* function)
  {
    IList<ir::Instr> fills;
    anton::Flat_Hash_Set<ir::Block*> visited(ctx.allocator);
    process_blocks(ctx, function->entry_block, visited, fills);
    ir::Instr* current = *fills.begin();
    ir::Instr* end = *fills.end();
    while(current != end) {
      ir::Instr* instr = current;
      current = current->next;
      if(instr->referrers.size() <= 0) {
        continue;
      }

      // ANTON_ASSERT(instr->referrers.size() == 1, "referrers not equal one");
      ir::Instr* const target = instr->referrers[0];
      ilist_insert_before(target, instr);
    }
  }
} // namespace glang
