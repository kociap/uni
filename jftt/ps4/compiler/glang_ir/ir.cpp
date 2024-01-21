#include <glang_ir/ir.hpp>

#include <glang_core/memory.hpp>

namespace glang::ir {
#define REPLACE(target)    \
  if(target == original) { \
    target = replacement;  \
  }

  static void replace_uses_with(Instr* const user, Instr* const original,
                                Instr* const replacement)
  {
    replacement->add_referrer(user);
    original->remove_referrer(user);
    switch(user->instr_kind) {
    case Instr_Kind::getelementptr: {
      auto const instr = static_cast<Instr_getelementptr*>(user);
      REPLACE(instr->offset);
    } break;

    case Instr_Kind::getelementptr_indirect: {
      auto const instr = static_cast<Instr_getelementptr_indirect*>(user);
      REPLACE(instr->offset);
    } break;

    case Instr_Kind::store: {
      auto const instr = static_cast<Instr_store*>(user);
      REPLACE(instr->dst);
      REPLACE(instr->src);
    } break;

    case Instr_Kind::store_indirect: {
      auto const instr = static_cast<Instr_store_indirect*>(user);
      REPLACE(instr->dst);
      REPLACE(instr->src);
    } break;

    case Instr_Kind::load: {
      auto const instr = static_cast<Instr_load*>(user);
      REPLACE(instr->src);
    } break;

    case Instr_Kind::load_indirect: {
      auto const instr = static_cast<Instr_load_indirect*>(user);
      REPLACE(instr->src);
    } break;

    case Instr_Kind::load_rhs: {
      auto const instr = static_cast<Instr_load_rhs*>(user);
      REPLACE(instr->src);
    } break;

    case Instr_Kind::alu: {
      auto const instr = static_cast<Instr_ALU*>(user);
      REPLACE(instr->lhs);
      REPLACE(instr->rhs);
    } break;

    case Instr_Kind::brcond: {
      auto const instr = static_cast<Instr_brcond*>(user);
      REPLACE(instr->condition);
    } break;

    case Instr_Kind::call: {
      auto const instr = static_cast<Instr_call*>(user);
      for(Instr*& arg: instr->arguments) {
        REPLACE(arg);
      }
    } break;

    case Instr_Kind::write: {
      auto const instr = static_cast<Instr_write*>(user);
      REPLACE(instr->src);
    } break;

    case Instr_Kind::read:
    case Instr_Kind::end:
    case Instr_Kind::ret:
    case Instr_Kind::constant:
    case Instr_Kind::variable:
    case Instr_Kind::branch:
      // Does not refer to any other instructions.
      break;
    }
  }

  void replace_uses_with(Instr* const instruction, Instr* const replacement)
  {
    for(Instr* const referrer: instruction->referrers) {
      replace_uses_with(referrer, instruction, replacement);
    }
  }

  bool is_branch(Instr const* instruction)
  {
    return instruction->instr_kind == Instr_Kind::end ||
           instruction->instr_kind == Instr_Kind::ret ||
           instruction->instr_kind == Instr_Kind::branch ||
           instruction->instr_kind == Instr_Kind::brcond;
  }

  Instr_variable* make_instr_variable(Context& ctx,
                                      Source_Info const& source_info,
                                      Variable_Kind const kind,
                                      anton::String_View identifier)
  {
    Instr_variable* const instr = allocate<Instr_variable>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), kind,
      anton::String(identifier, ctx.allocator), source_info);
    return instr;
  }

  Instr_constant* make_instr_constant(Context& ctx,
                                      Source_Info const& source_info,
                                      i64 const value)
  {
    Instr_constant* const instr = allocate<Instr_constant>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), value, source_info);
    return instr;
  }

  Instr_getelementptr* make_instr_getelementptr(Context& ctx,
                                                Source_Info const& source_info,
                                                Instr_variable* variable,
                                                Instr* offset)
  {
    Instr_getelementptr* const instr = allocate<Instr_getelementptr>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), variable, offset,
      source_info);
    variable->add_referrer(instr);
    offset->add_referrer(instr);
    return instr;
  }

  Instr_getelementptr_indirect*
  make_instr_getelementptr_indirect(Context& ctx,
                                    Source_Info const& source_info,
                                    Instr_variable* variable, Instr* offset)
  {
    Instr_getelementptr_indirect* const instr =
      allocate<Instr_getelementptr_indirect>(ctx.allocator, ctx.allocator,
                                             ctx.get_next_id(), variable,
                                             offset, source_info);
    variable->add_referrer(instr);
    offset->add_referrer(instr);
    return instr;
  }

  Instr_store* make_instr_store(Context& ctx, Source_Info const& source_info,
                                Instr* const dst, Instr* const src)
  {
    Instr_store* const instr = allocate<Instr_store>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), dst, src, source_info);
    dst->add_referrer(instr);
    src->add_referrer(instr);
    return instr;
  }

  Instr_store_indirect*
  make_instr_store_indirect(Context& ctx, Source_Info const& source_info,
                            Instr* dst, Instr* src)
  {
    Instr_store_indirect* const instr = allocate<Instr_store_indirect>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), dst, src, source_info);
    dst->add_referrer(instr);
    src->add_referrer(instr);
    return instr;
  }

  Instr_load* make_instr_load(Context& ctx, Source_Info const& source_info,
                              Instr* const src)
  {
    Instr_load* const instr = allocate<Instr_load>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), src, source_info);
    src->add_referrer(instr);
    return instr;
  }

  Instr_load_indirect* make_instr_load_indirect(Context& ctx,
                                                Source_Info const& source_info,
                                                Instr* src)
  {
    Instr_load_indirect* const instr = allocate<Instr_load_indirect>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), src, source_info);
    src->add_referrer(instr);
    return instr;
  }

  Instr_load_rhs*
  make_instr_load_rhs(Context& ctx, Source_Info const& source_info, Instr* src)
  {
    Instr_load_rhs* const instr = allocate<Instr_load_rhs>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), src, source_info);
    src->add_referrer(instr);
    return instr;
  }

  Instr_ALU* make_instr_alu(Context& ctx, Source_Info const& source_info,
                            ALU_Opcode const opcode, Instr* const lhs,
                            Instr* const rhs)
  {
    Instr_ALU* const instr =
      allocate<Instr_ALU>(ctx.allocator, ctx.allocator, ctx.get_next_id(),
                          opcode, lhs, rhs, source_info);
    lhs->add_referrer(instr);
    rhs->add_referrer(instr);
    return instr;
  }

  Instr_end* make_instr_end(Context& ctx, Source_Info const& source_info)
  {
    Instr_end* const instr = allocate<Instr_end>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), source_info);
    return instr;
  }

  Instr_ret* make_instr_ret(Context& ctx, Source_Info const& source_info)
  {
    Instr_ret* const instr = allocate<Instr_ret>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), source_info);
    return instr;
  }

  Instr_branch* make_instr_branch(Context& ctx, Source_Info const& source_info,
                                  Block* const target)
  {
    Instr_branch* const instr = allocate<Instr_branch>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), target, source_info);
    return instr;
  }

  Instr_brcond* make_instr_brcond(Context& ctx, Source_Info const& source_info,
                                  Brcond_Kind const kind,
                                  Instr* const condition,
                                  Block* const then_target,
                                  Block* const else_target)
  {
    Instr_brcond* const instr = allocate<Instr_brcond>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), kind, condition,
      then_target, else_target, source_info);
    condition->add_referrer(instr);
    return instr;
  }

  Instr_call* make_instr_call(Context& ctx, Source_Info const& source_info,
                              Function* function)
  {
    Instr_call* const instr = allocate<Instr_call>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), function, source_info);
    return instr;
  }

  Instr_read* make_instr_read(Context& ctx, Source_Info const& source_info)
  {
    Instr_read* const instr = allocate<Instr_read>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), source_info);
    return instr;
  }

  Instr_write* make_instr_write(Context& ctx, Source_Info const& source_info,
                                Instr* const src)
  {
    Instr_write* const instr = allocate<Instr_write>(
      ctx.allocator, ctx.allocator, ctx.get_next_id(), src, source_info);
    src->add_referrer(instr);
    return instr;
  }

  Block* make_block(Context& ctx, Function* function)
  {
    Block* const block =
      allocate<Block>(ctx.allocator, function, ctx.get_next_block_label());
    return block;
  }

  Block* split_block(Context& ctx, Function* const function, i64 const label,
                     Instr* instruction, Instr* const end, bool const before)
  {
    Block* const new_block = allocate<Block>(ctx.allocator, function, label);
    Instr_branch* const branch =
      make_instr_branch(ctx, instruction->source_info, new_block);
    if(before) {
      ilist_insert_before(instruction, branch);
    } else {
      ilist_insert_after(instruction, branch);
    }

    ilist_splice(*new_block->begin(), branch->next, end);

    return new_block;
  }
} // namespace glang::ir
