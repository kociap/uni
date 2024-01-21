#include "glang_codegen_vm/lir.hpp"
#include "glang_core/source_info.hpp"
#include "glang_ir/opcodes.hpp"
#include <glang_codegen_vm/ir_lowering.hpp>

#include <anton/flat_hash_map.hpp>
#include <anton/flat_hash_set.hpp>
#include <anton/format.hpp>
#include <anton/stdio.hpp>

#include <glang_codegen_vm/context.hpp>
#include <glang_core/ilist.hpp>
#include <glang_core/iterators.hpp>
#include <glang_core/memory.hpp>
#include <glang_ir/ir.hpp>

namespace glang {
  struct Call_Replacement: public IList_Node<Call_Replacement> {
    lir::Instr_jump* call;
    ir::Function* function;
  };

  [[nodiscard]] static Call_Replacement*
  make_call_replacement(lir::Context& ctx, lir::Instr_jump* call,
                        ir::Function* function)
  {
    return allocate<Call_Replacement>(
      ctx.allocator, Call_Replacement{.call = call, .function = function});
  }

  struct Jump_Replacement: public IList_Node<Jump_Replacement> {
    lir::Instr* jump;
    ir::Block* block;
  };

  [[nodiscard]] static Jump_Replacement*
  make_jump_replacement(lir::Context& ctx, lir::Instr* jump, ir::Block* block)
  {
    return allocate<Jump_Replacement>(
      ctx.allocator, Jump_Replacement{.jump = jump, .block = block});
  }

  struct Builder {
    IList<lir::Instr> instructions;
    IList<Call_Replacement> call_replacements;
    IList<Jump_Replacement> jump_replacements;
    lir::Instr* block_first_instr = nullptr;
    bool block_begin = false;

    void begin_block()
    {
      block_begin = true;
    }

    void insert(lir::Instr* instruction)
    {
      if(block_begin) {
        block_first_instr = instruction;
        block_begin = false;
      }
      instructions.insert_back(instruction);
    }

    void add_pending_replacement(Call_Replacement* replacement)
    {
      call_replacements.insert_back(replacement);
    }

    void add_pending_replacement(Jump_Replacement* replacement)
    {
      jump_replacements.insert_back(replacement);
    }
  };

  void generate_constant(lir::Context& ctx, Builder& builder,
                         Source_Info const& source_info, i64 const value,
                         lir::Reg_Kind const reg = lir::Reg_Kind::a)
  {
    lir::Reg* zero_reg = lir::make_reg(ctx, reg);
    auto const zero = lir::make_instr_rst(ctx, zero_reg, source_info);
    builder.insert(zero);
    if(value > 0) {
      lir::Reg* warmup_reg_a = lir::make_reg(ctx, reg);
      lir::Instr* warmup_inc =
        lir::make_instr_inc(ctx, warmup_reg_a, source_info);
      builder.insert(warmup_inc);
      i64 const first_bit = 64 - anton::math::clz((u64)value);
      for(i64 offset = first_bit - 2; offset >= 0; offset -= 1) {
        lir::Reg* reg_shl = lir::make_reg(ctx, reg);
        lir::Instr* shl = lir::make_instr_shl(ctx, reg_shl, source_info);
        builder.insert(shl);
        if((value & (1 << offset)) != 0) {
          lir::Reg* reg_inc = lir::make_reg(ctx, reg);
          lir::Instr* inc = lir::make_instr_inc(ctx, reg_inc, source_info);
          builder.insert(inc);
        }
      }
    }
  }

  template<typename T>
  T* make_instr(lir::Context& ctx, Builder& builder, lir::Reg_Kind kind,
                Source_Info const& source_info)
  {
    lir::Reg* reg = lir::make_reg(ctx, kind);
    auto const instr = allocate<T>(ctx.allocator, reg, source_info);
    builder.insert(instr);
    return instr;
  }

#define MAKE_INSTR(T, reg) \
  make_instr<T>(ctx, builder, lir::Reg_Kind::reg, source_info)

  void generate_multiplication(lir::Context& ctx, Builder& builder,
                               Source_Info const& source_info)
  {
    // Reg c is the result.

    MAKE_INSTR(lir::Instr_rst, c);
    auto const jzero_end = lir::make_instr_jzero(ctx, nullptr, source_info);
    builder.insert(jzero_end);
    MAKE_INSTR(lir::Instr_put, d);
    MAKE_INSTR(lir::Instr_shr, d);
    MAKE_INSTR(lir::Instr_shl, d);
    MAKE_INSTR(lir::Instr_put, e);
    MAKE_INSTR(lir::Instr_sub, d);
    auto const jzero_loop2 = lir::make_instr_jzero(ctx, nullptr, source_info);
    builder.insert(jzero_loop2);
    MAKE_INSTR(lir::Instr_get, c);
    MAKE_INSTR(lir::Instr_add, b);
    MAKE_INSTR(lir::Instr_put, c);
    auto const loop2 = MAKE_INSTR(lir::Instr_get, e);
    MAKE_INSTR(lir::Instr_shr, a);
    MAKE_INSTR(lir::Instr_shl, b);
    auto const jump_cond = lir::make_instr_jump(ctx, nullptr, source_info);
    builder.insert(jump_cond);
    auto const end = MAKE_INSTR(lir::Instr_get, c);
    jzero_end->target = end;
    end->add_incoming(jzero_end);
    jzero_loop2->target = loop2;
    loop2->add_incoming(jzero_loop2);
    jump_cond->target = jzero_end;
    jzero_end->add_incoming(jump_cond);
  }

  void generate_divrem(lir::Context& ctx, Builder& builder,
                       Source_Info const& source_info, bool remainder = false)
  {
    // Reg c is the result.
    // Reg e is the remainder.

    MAKE_INSTR(lir::Instr_rst, c);
    MAKE_INSTR(lir::Instr_rst, e);
    // Guard against division by 0.
    MAKE_INSTR(lir::Instr_put, f);
    MAKE_INSTR(lir::Instr_get, b);
    auto const jzero_end = lir::make_instr_jzero(ctx, nullptr, source_info);
    builder.insert(jzero_end);
    MAKE_INSTR(lir::Instr_get, f);
    // Division.
    auto const cond = MAKE_INSTR(lir::Instr_put, e);
    MAKE_INSTR(lir::Instr_get, b);
    MAKE_INSTR(lir::Instr_sub, e);
    auto const jpos_end = lir::make_instr_jpos(ctx, nullptr, source_info);
    builder.insert(jpos_end);
    MAKE_INSTR(lir::Instr_rst, f);
    MAKE_INSTR(lir::Instr_inc, f);
    MAKE_INSTR(lir::Instr_get, b);
    MAKE_INSTR(lir::Instr_put, d);
    auto const innerloopcond = MAKE_INSTR(lir::Instr_get, d);
    MAKE_INSTR(lir::Instr_sub, e);
    auto const jpos_loop2 = lir::make_instr_jpos(ctx, nullptr, source_info);
    builder.insert(jpos_loop2);
    MAKE_INSTR(lir::Instr_shl, f);
    MAKE_INSTR(lir::Instr_shl, d);
    auto const jump_innerloopcond =
      lir::make_instr_jump(ctx, nullptr, source_info);
    builder.insert(jump_innerloopcond);
    auto const loop2 = MAKE_INSTR(lir::Instr_shr, f);
    MAKE_INSTR(lir::Instr_shr, d);
    MAKE_INSTR(lir::Instr_get, c);
    MAKE_INSTR(lir::Instr_add, f);
    MAKE_INSTR(lir::Instr_put, c);
    MAKE_INSTR(lir::Instr_get, e);
    MAKE_INSTR(lir::Instr_sub, d);
    auto const jump_cond = lir::make_instr_jump(ctx, nullptr, source_info);
    builder.insert(jump_cond);
    auto const end = !remainder ? MAKE_INSTR(lir::Instr_get, c)
                                : MAKE_INSTR(lir::Instr_get, e);

    jzero_end->target = end;

    jpos_end->target = end;
    end->add_incoming(jpos_end);
    jpos_loop2->target = loop2;
    loop2->add_incoming(jpos_loop2);
    jump_innerloopcond->target = innerloopcond;
    innerloopcond->add_incoming(jump_innerloopcond);
    jump_cond->target = cond;
    cond->add_incoming(jump_cond);
  }

  void generate_neq(lir::Context& ctx, Builder& builder,
                    Source_Info const& source_info)
  {
    MAKE_INSTR(lir::Instr_put, e);
    MAKE_INSTR(lir::Instr_sub, b);
    auto const jpos_end = lir::make_instr_jpos(ctx, nullptr, source_info);
    builder.insert(jpos_end);
    MAKE_INSTR(lir::Instr_get, b);
    MAKE_INSTR(lir::Instr_sub, e);
    auto const end = MAKE_INSTR(lir::Instr_get, a); // noop

    jpos_end->target = end;
    end->add_incoming(jpos_end);
  }

  void generate_lt(lir::Context& ctx, Builder& builder,
                   Source_Info const& source_info)
  {
    MAKE_INSTR(lir::Instr_put, e);
    MAKE_INSTR(lir::Instr_get, b);
    MAKE_INSTR(lir::Instr_sub, e);
  }

  void generate_gt(lir::Context& ctx, Builder& builder,
                   Source_Info const& source_info)
  {
    MAKE_INSTR(lir::Instr_sub, b);
  }

  void lower_block(lir::Context& ctx, Builder& builder,
                   ir::Block const* const block,
                   lir::Variable const* const return_address)
  {
    builder.begin_block();

    for(ir::Instr const* const generic_instr: *block) {
      switch(generic_instr->instr_kind) {
      case ir::Instr_Kind::constant: {
        auto const instr =
          static_cast<ir::Instr_constant const*>(generic_instr);
        generate_constant(ctx, builder, instr->source_info, instr->value);
      } break;

      case ir::Instr_Kind::getelementptr: {
        auto const instr =
          static_cast<ir::Instr_getelementptr const*>(generic_instr);
        Source_Info const source_info = instr->source_info;
        auto iter = ctx.variables.find(instr->variable->id);
        ANTON_ASSERT(iter != ctx.variables.end(), "missing variable");
        // RHS should be in reg a. Generate the address into reg b to avoid mov.
        generate_constant(ctx, builder, instr->source_info,
                          iter->value->address, lir::Reg_Kind::b);
        MAKE_INSTR(lir::Instr_add, b);
      } break;

      case ir::Instr_Kind::getelementptr_indirect: {
        auto const instr =
          static_cast<ir::Instr_getelementptr_indirect const*>(generic_instr);
        Source_Info const source_info = instr->source_info;
        auto iter = ctx.variables.find(instr->variable->id);
        ANTON_ASSERT(iter != ctx.variables.end(), "missing variable");
        // RHS should be in reg a. Since we have to load the address, we move it
        // to reg b.
        MAKE_INSTR(lir::Instr_put, b);
        generate_constant(ctx, builder, instr->source_info,
                          iter->value->address, lir::Reg_Kind::a);
        MAKE_INSTR(lir::Instr_load, a);
        MAKE_INSTR(lir::Instr_add, b);
      } break;

      case ir::Instr_Kind::store_indirect: {
      } break;

      case ir::Instr_Kind::store: {
        auto const instr = static_cast<ir::Instr_store const*>(generic_instr);
        auto iter = ctx.variables.find(instr->dst->id);
        if(iter == ctx.variables.end()) {
          // We are storing to an address in the register b.
          lir::Reg* const reg_b = lir::make_reg(ctx, lir::Reg_Kind::b);
          lir::Instr* const store =
            lir::make_instr_store(ctx, reg_b, instr->source_info);
          builder.insert(store);
        } else {
          generate_constant(ctx, builder, instr->source_info,
                            iter->value->address, lir::Reg_Kind::b);
          lir::Reg* reg = lir::make_reg(ctx, lir::Reg_Kind::b);
          lir::Instr* store =
            lir::make_instr_store(ctx, reg, generic_instr->source_info);
          // lir::Instr* store = lir::make_instr_store(
          //   ctx, (lir::Reg*)iter->value->address, generic_instr->source_info);
          builder.insert(store);
        }
      } break;

      case ir::Instr_Kind::load: {
        auto const instr = static_cast<ir::Instr_load const*>(generic_instr);
        Source_Info const source_info = instr->source_info;
        auto iter = ctx.variables.find(instr->src->id);
        if(iter == ctx.variables.end()) {
          // We are loading from an address in the register a.
          MAKE_INSTR(lir::Instr_load, a);
        } else {
          // Move reg a to reg b in case there's a value there.
          MAKE_INSTR(lir::Instr_put, b);
          generate_constant(ctx, builder, instr->source_info,
                            iter->value->address);
          MAKE_INSTR(lir::Instr_load, a);
          // lir::Instr* load = lir::make_instr_load(
          //   ctx, (lir::Reg*)iter->value->address, generic_instr->source_info);
        }
      } break;

      case ir::Instr_Kind::load_indirect: {
        auto const instr = static_cast<ir::Instr_load const*>(generic_instr);
        // Move reg a to reg b in case there's a value there.
        lir::Reg* put_reg = lir::make_reg(ctx, lir::Reg_Kind::b);
        lir::Instr* put = lir::make_instr_put(ctx, put_reg, instr->source_info);
        builder.insert(put);
        auto iter = ctx.variables.find(instr->src->id);
        ANTON_ASSERT(iter != ctx.variables.end(), "missing variable");
        generate_constant(ctx, builder, instr->source_info,
                          iter->value->address);
        lir::Reg* reg1 = lir::make_reg(ctx, lir::Reg_Kind::a);
        lir::Instr* load1 =
          lir::make_instr_load(ctx, reg1, generic_instr->source_info);
        builder.insert(load1);
        lir::Reg* reg2 = lir::make_reg(ctx, lir::Reg_Kind::a);
        lir::Instr* load2 =
          lir::make_instr_load(ctx, reg2, generic_instr->source_info);
        builder.insert(load2);
      } break;

      case ir::Instr_Kind::load_rhs: {
        auto const instr =
          static_cast<ir::Instr_load_rhs const*>(generic_instr);
        Source_Info const source_info = instr->source_info;
        // Move reg a to reg h.
        MAKE_INSTR(lir::Instr_put, h);
        // Generate load address.
        auto iter = ctx.variables.find(instr->src->id);
        ANTON_ASSERT(iter != ctx.variables.end(), "missing variable");
        generate_constant(ctx, builder, instr->source_info,
                          iter->value->address);
        MAKE_INSTR(lir::Instr_load, a);
        MAKE_INSTR(lir::Instr_put, b);
        MAKE_INSTR(lir::Instr_get, h);
      } break;

      case ir::Instr_Kind::alu: {
        auto const instr = static_cast<ir::Instr_ALU const*>(generic_instr);
        switch(instr->opcode) {
        case ir::ALU_Opcode::add: {
          lir::Reg* reg = lir::make_reg(ctx, lir::Reg_Kind::b);
          lir::Instr* add = lir::make_instr_add(ctx, reg, instr->source_info);
          builder.insert(add);
        } break;

        case ir::ALU_Opcode::sub: {
          lir::Reg* reg = lir::make_reg(ctx, lir::Reg_Kind::b);
          lir::Instr* sub = lir::make_instr_sub(ctx, reg, instr->source_info);
          builder.insert(sub);
        } break;

        case ir::ALU_Opcode::shl: {
          lir::Reg* reg = lir::make_reg(ctx, lir::Reg_Kind::a);
          lir::Instr* shl = lir::make_instr_shl(ctx, reg, instr->source_info);
          builder.insert(shl);
        } break;

        case ir::ALU_Opcode::shr: {
          lir::Reg* reg = lir::make_reg(ctx, lir::Reg_Kind::a);
          lir::Instr* shr = lir::make_instr_shr(ctx, reg, instr->source_info);
          builder.insert(shr);
        } break;

        case ir::ALU_Opcode::mul: {
          generate_multiplication(ctx, builder, instr->source_info);
        } break;

        case ir::ALU_Opcode::div: {
          generate_divrem(ctx, builder, instr->source_info, false);
        } break;

        case ir::ALU_Opcode::mod: {
          generate_divrem(ctx, builder, instr->source_info, true);
        } break;

        case ir::ALU_Opcode::neq: {
          generate_neq(ctx, builder, instr->source_info);
        } break;

        case ir::ALU_Opcode::lt: {
          generate_lt(ctx, builder, instr->source_info);
        } break;

        case ir::ALU_Opcode::gt: {
          generate_gt(ctx, builder, instr->source_info);
        } break;

        default:
          ANTON_UNREACHABLE("invalid alu opcode");
        }
      } break;

      case ir::Instr_Kind::end: {
        lir::Instr* const instr =
          lir::make_instr_halt(ctx, generic_instr->source_info);
        builder.insert(instr);
      } break;

      case ir::Instr_Kind::ret: {
        auto const instr = static_cast<ir::Instr_ret const*>(generic_instr);
        generate_constant(ctx, builder, instr->source_info,
                          return_address->address);
        lir::Reg* reg1_a = lir::make_reg(ctx, lir::Reg_Kind::a);
        lir::Instr* const load =
          lir::make_instr_load(ctx, reg1_a, instr->source_info);
        builder.insert(load);
        // Callsite executes 2 instructions (strk and jump) to perform a call,
        // thus the return address is what strk stored plus 2.
        lir::Reg* reg3_a = lir::make_reg(ctx, lir::Reg_Kind::a);
        lir::Instr* const inc1 =
          lir::make_instr_inc(ctx, reg3_a, instr->source_info);
        builder.insert(inc1);
        lir::Reg* reg4_a = lir::make_reg(ctx, lir::Reg_Kind::a);
        lir::Instr* const inc2 =
          lir::make_instr_inc(ctx, reg4_a, instr->source_info);
        builder.insert(inc2);
        lir::Reg* reg2_a = lir::make_reg(ctx, lir::Reg_Kind::a);
        lir::Instr* const jumpr =
          lir::make_instr_jumpr(ctx, reg2_a, instr->source_info);
        builder.insert(jumpr);
      } break;

      case ir::Instr_Kind::branch: {
        auto const instr = static_cast<ir::Instr_branch const*>(generic_instr);
        lir::Instr* const jump =
          lir::make_instr_jump(ctx, nullptr, instr->source_info);
        builder.insert(jump);
        builder.add_pending_replacement(
          make_jump_replacement(ctx, jump, instr->target));
      } break;

      case ir::Instr_Kind::brcond: {
        auto const instr = static_cast<ir::Instr_brcond const*>(generic_instr);
        switch(instr->kind) {
        case ir::Brcond_Kind::jgt: {
          lir::Instr* const jpos =
            lir::make_instr_jpos(ctx, nullptr, instr->source_info);
          builder.insert(jpos);
          builder.add_pending_replacement(
            make_jump_replacement(ctx, jpos, instr->then_target));
          lir::Instr* const jump =
            lir::make_instr_jump(ctx, nullptr, instr->source_info);
          builder.insert(jump);
          builder.add_pending_replacement(
            make_jump_replacement(ctx, jump, instr->else_target));
        } break;

        case ir::Brcond_Kind::jz: {
          lir::Instr* const jzero =
            lir::make_instr_jzero(ctx, nullptr, instr->source_info);
          builder.insert(jzero);
          builder.add_pending_replacement(
            make_jump_replacement(ctx, jzero, instr->then_target));
          lir::Instr* const jump =
            lir::make_instr_jump(ctx, nullptr, instr->source_info);
          builder.insert(jump);
          builder.add_pending_replacement(
            make_jump_replacement(ctx, jump, instr->else_target));
        } break;
        }
      } break;

      case ir::Instr_Kind::call: {
        auto const instr = static_cast<ir::Instr_call const*>(generic_instr);
        for(auto [parameter, garg]:
            zip(instr->function->parameters, instr->arguments)) {
          ir::Instr_variable* arg = static_cast<ir::Instr_variable*>(garg);
          auto parameter_iter = ctx.variables.find(parameter->id);
          ANTON_ASSERT(parameter_iter != ctx.variables.end(),
                       "parameter has no addressable variable");
          auto argument_iter = ctx.variables.find(arg->id);
          ANTON_ASSERT(argument_iter != ctx.variables.end(),
                       "argument has no addressable variable");
          lir::Variable* const parameter_var = parameter_iter->value;
          lir::Variable* const argument_var = argument_iter->value;
          if(argument_var->pointer) {
            generate_constant(ctx, builder, instr->source_info,
                              argument_var->address);
            lir::Reg* const indirect_reg = lir::make_reg(ctx, lir::Reg_Kind::a);
            lir::Instr* const load =
              lir::make_instr_load(ctx, indirect_reg, instr->source_info);
            builder.insert(load);
            generate_constant(ctx, builder, instr->source_info,
                              parameter_var->address, lir::Reg_Kind::b);
            lir::Reg* const store_reg = lir::make_reg(ctx, lir::Reg_Kind::b);
            lir::Instr* const store =
              lir::make_instr_store(ctx, store_reg, instr->source_info);
            builder.insert(store);
          } else {
            generate_constant(ctx, builder, instr->source_info,
                              parameter_var->address, lir::Reg_Kind::b);
            generate_constant(ctx, builder, instr->source_info,
                              argument_var->address);
            lir::Reg* const store_reg = lir::make_reg(ctx, lir::Reg_Kind::b);
            lir::Instr* const store =
              lir::make_instr_store(ctx, store_reg, instr->source_info);
            builder.insert(store);
          }
        }
        lir::Instr* const strk = lir::make_instr_strk(ctx, instr->source_info);
        builder.insert(strk);
        lir::Instr_jump* const jump =
          lir::make_instr_jump(ctx, nullptr, instr->source_info);
        builder.insert(jump);
        builder.add_pending_replacement(
          make_call_replacement(ctx, jump, instr->function));
      } break;

      case ir::Instr_Kind::read: {
        auto const instr = static_cast<ir::Instr_read const*>(generic_instr);
        // Move reg a to reg b in case there's a value there.
        lir::Reg* put_reg = lir::make_reg(ctx, lir::Reg_Kind::b);
        lir::Instr* put = lir::make_instr_put(ctx, put_reg, instr->source_info);
        builder.insert(put);
        lir::Instr* const read = lir::make_instr_read(ctx, instr->source_info);
        builder.insert(read);
      } break;

      case ir::Instr_Kind::write: {
        auto const instr = static_cast<ir::Instr_write const*>(generic_instr);
        lir::Instr* const write =
          lir::make_instr_write(ctx, instr->source_info);
        builder.insert(write);
      } break;

      case ir::Instr_Kind::variable:
        ANTON_UNREACHABLE("illegal instr_variable in block");
      }
    }

    lir::Block_Data block_data{.first_instr = builder.block_first_instr};
    ctx.blocks.emplace(block, block_data);
  }

  static void process_blocks(lir::Context& ctx, Builder& builder,
                             ir::Block* const block,
                             lir::Variable* const return_address,
                             anton::Flat_Hash_Set<ir::Block const*>& visited)
  {
    if(visited.find(block) != visited.end()) {
      return;
    }

    visited.emplace(block);
    lower_block(ctx, builder, block, return_address);
    ir::Instr* const terminator = block->get_last();
    ANTON_ASSERT(ir::is_branch(terminator), "terminator is not a branch");
    if(terminator->instr_kind == ir::Instr_Kind::branch) {
      auto const instr = static_cast<ir::Instr_branch*>(terminator);
      process_blocks(ctx, builder, instr->target, return_address, visited);
    } else if(terminator->instr_kind == ir::Instr_Kind::brcond) {
      auto const instr = static_cast<ir::Instr_brcond*>(terminator);
      process_blocks(ctx, builder, instr->then_target, return_address, visited);
      process_blocks(ctx, builder, instr->else_target, return_address, visited);
    }
  }

  static void build_stackframe(lir::Context& ctx,
                               ir::Function const* const function)
  {
    for(ir::Instr_variable const* local: function->locals) {
      lir::Variable* lir_local = ctx.allocate_variable(local->size, false);
      ctx.variables.emplace(local->id, lir_local);
    }

    if(!function->entry) {
      for(ir::Instr_variable const* local: function->parameters) {
        lir::Variable* lir_local = ctx.allocate_variable(local->size, true);
        ctx.variables.emplace(local->id, lir_local);
      }
    }
  }

  struct Fn_Lower_Result {
    IList<lir::Instr> instructions;
    lir::Variable* return_address;
    IList<Call_Replacement> call_replacements;
    IList<Jump_Replacement> jump_replacements;
  };

  [[nodiscard]] static Fn_Lower_Result
  lower_function(lir::Context& ctx, ir::Function const* const function)
  {
    Builder builder;
    anton::Flat_Hash_Set<ir::Block const*> visited(ctx.allocator);
    lir::Variable* return_address = ctx.allocate_variable(1, false);
    // Store the return address from register a to memory.
    generate_constant(ctx, builder, function->source_info,
                      return_address->address, lir::Reg_Kind::b);
    lir::Reg* const store_reg = lir::make_reg(ctx, lir::Reg_Kind::b);
    lir::Instr* const store =
      lir::make_instr_store(ctx, store_reg, function->source_info);
    builder.insert(store);

    process_blocks(ctx, builder, function->entry_block, return_address,
                   visited);
    return Fn_Lower_Result{
      .instructions = ANTON_MOV(builder.instructions),
      .return_address = return_address,
      .call_replacements = ANTON_MOV(builder.call_replacements),
      .jump_replacements = ANTON_MOV(builder.jump_replacements)};
  }

  [[nodiscard]] static Fn_Lower_Result
  lower_entry(lir::Context& ctx, ir::Function const* const function)
  {
    Builder builder;
    anton::Flat_Hash_Set<ir::Block const*> visited(ctx.allocator);
    process_blocks(ctx, builder, function->entry_block, nullptr, visited);
    return Fn_Lower_Result{
      .instructions = ANTON_MOV(builder.instructions),
      .return_address = nullptr,
      .call_replacements = ANTON_MOV(builder.call_replacements),
      .jump_replacements = ANTON_MOV(builder.jump_replacements)};
  }

  IList<lir::Instr> lower_ir_to_lir(Allocator* const allocator,
                                    IList<ir::Function> const& functions)
  {
    lir::Context ctx{
      .functions = {allocator}, .blocks = {allocator}, .allocator = allocator};

    for(ir::Function const* fn: functions) {
      build_stackframe(ctx, fn);
    }

    for(auto [irid, variable]: ctx.variables) {
      anton::print(
        anton::format(allocator, "%{} = {}\n", irid, variable->address));
    }

    IList<lir::Instr> program_instructions;
    IList<Jump_Replacement> jump_replacements;
    IList<Call_Replacement> call_replacements;
    for(ir::Function const* const function: functions) {
      if(function->entry) {
        Fn_Lower_Result result = lower_entry(ctx, function);
        auto location = program_instructions.splice(
          program_instructions.begin(), result.instructions);
        lir::Function_Data fn_data{.first_instr = *location,
                                   .return_address = nullptr};
        ctx.functions.emplace(function, fn_data);
        jump_replacements.splice(jump_replacements.end(),
                                 result.jump_replacements);
        call_replacements.splice(call_replacements.end(),
                                 result.call_replacements);
      } else {
        Fn_Lower_Result result = lower_function(ctx, function);
        auto location = program_instructions.splice(
          program_instructions.begin(), result.instructions);
        lir::Function_Data fn_data{.first_instr = *location,
                                   .return_address = result.return_address};
        ctx.functions.emplace(function, fn_data);
        jump_replacements.splice(jump_replacements.end(),
                                 result.jump_replacements);
        call_replacements.splice(call_replacements.end(),
                                 result.call_replacements);
      }
    }

    // Resolve pending replacements.
    for(Jump_Replacement* replacement: jump_replacements) {
      auto iter = ctx.blocks.find(replacement->block);
      ANTON_ASSERT(iter != ctx.blocks.end(), "replacement has no block");
      lir::Block_Data& data = iter->value;
      lir::Instr* const jump = replacement->jump;
      switch(jump->instr_kind) {
      case lir::Instr_Kind::jump: {
        auto const instr = static_cast<lir::Instr_jump*>(jump);
        instr->target = data.first_instr;
        data.first_instr->add_incoming(instr);
      } break;

      case lir::Instr_Kind::jpos: {
        auto const instr = static_cast<lir::Instr_jpos*>(jump);
        instr->target = data.first_instr;
        data.first_instr->add_incoming(instr);
      } break;

      case lir::Instr_Kind::jzero: {
        auto const instr = static_cast<lir::Instr_jzero*>(jump);
        instr->target = data.first_instr;
        data.first_instr->add_incoming(instr);
      } break;

      default:
        ANTON_UNREACHABLE("invalid jump kind");
      }
    }

    for(Call_Replacement* replacement: call_replacements) {
      auto iter = ctx.functions.find(replacement->function);
      ANTON_ASSERT(iter != ctx.functions.end(), "replacement has no function");
      lir::Function_Data& data = iter->value;
      replacement->call->target = data.first_instr;
      data.first_instr->add_incoming(replacement->call);
    }

    return program_instructions;
  }
} // namespace glang
