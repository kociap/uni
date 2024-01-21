#include <glang_ir/prettyprint.hpp>

#include <anton/flat_hash_set.hpp>
#include <anton/format.hpp>
#include <anton/string_view.hpp>

namespace glang {
  using namespace anton::literals;

  struct PP_Context {
    Allocator* allocator;
    anton::Flat_Hash_Set<i64> visited_blocks;
  };

  static void print_indent(anton::Output_Stream& stream, i64 const indent)
  {
    for(i64 i = 0; i < indent; i += 1) {
      stream.write(" "_sv);
    }
  }

  static void print_block(PP_Context& ctx, anton::Output_Stream& stream,
                          ir::Block const* const block, i64 const indent);

  [[nodiscard]] static anton::String_View
  stringify(ir::Variable_Kind const kind)
  {
    switch(kind) {
    case ir::Variable_Kind::local:
      return "local"_sv;
    case ir::Variable_Kind::param:
      return "param"_sv;
    }
  }

  [[nodiscard]] static anton::String_View stringify(ir::ALU_Opcode const opcode)
  {
    switch(opcode) {
    case ir::ALU_Opcode::add:
      return "add"_sv;
    case ir::ALU_Opcode::sub:
      return "sub"_sv;
    case ir::ALU_Opcode::mul:
      return "mul"_sv;
    case ir::ALU_Opcode::div:
      return "div"_sv;
    case ir::ALU_Opcode::mod:
      return "mod"_sv;
    case ir::ALU_Opcode::eq:
      return "eq"_sv;
    case ir::ALU_Opcode::neq:
      return "neq"_sv;
    case ir::ALU_Opcode::lt:
      return "lt"_sv;
    case ir::ALU_Opcode::gt:
      return "gt"_sv;
    case ir::ALU_Opcode::leq:
      return "leq"_sv;
    case ir::ALU_Opcode::geq:
      return "geq"_sv;
    case ir::ALU_Opcode::shl:
      return "shl"_sv;
    case ir::ALU_Opcode::shr:
      return "shr"_sv;
    }
  }

  [[nodiscard]] static anton::String_View stringify(ir::Brcond_Kind const kind)
  {
    switch(kind) {
    case ir::Brcond_Kind::jgt:
      return "jgt"_sv;
    case ir::Brcond_Kind::jz:
      return "jz"_sv;
    }
  }

  [[nodiscard]] static anton::String
  stringify_referrers(Allocator* allocator,
                      anton::Slice<ir::Instr* const> const referrers)
  {
    anton::String str(allocator);
    for(ir::Instr* const referrer: referrers) {
      str += anton::format(allocator, " %{}", referrer->id);
    }
    return str;
  }

  static void print_instr(PP_Context& ctx, anton::Output_Stream& stream,
                          ir::Instr const* const generic_instr,
                          i64 const indent)
  {
    print_indent(stream, indent);
    switch(generic_instr->instr_kind) {
    case ir::Instr_Kind::variable: {
      auto const instr = static_cast<ir::Instr_variable const*>(generic_instr);
      anton::String_View const kind = stringify(instr->kind);
      anton::String const referrers_str =
        stringify_referrers(ctx.allocator, instr->referrers);
      stream.write(anton::format(
        ctx.allocator,
        "%{} = variable {} \"{}\" x{} [referrers {}:{}] @ {}:{}\n"_sv,
        instr->id, kind, instr->identifier, instr->size,
        instr->referrers.size(), referrers_str, instr->source_info.line,
        instr->source_info.column));
    } break;

    case ir::Instr_Kind::constant: {
      auto const instr = static_cast<ir::Instr_constant const*>(generic_instr);
      anton::String const referrers_str =
        stringify_referrers(ctx.allocator, instr->referrers);
      stream.write(anton::format(
        ctx.allocator, "%{} = constant {} [referrers {}:{}] @ {}:{}\n"_sv,
        instr->id, instr->value, instr->referrers.size(), referrers_str,
        instr->source_info.line, instr->source_info.column));
    } break;

    case ir::Instr_Kind::getelementptr: {
      auto const instr =
        static_cast<ir::Instr_getelementptr const*>(generic_instr);
      anton::String const referrers_str =
        stringify_referrers(ctx.allocator, instr->referrers);
      stream.write(anton::format(
        ctx.allocator,
        "%{} = getelementptr %{} %{} [referrers {}:{}] @ {}:{}\n"_sv, instr->id,
        instr->variable->id, instr->offset->id, instr->referrers.size(),
        referrers_str, instr->source_info.line, instr->source_info.column));
    } break;

    case ir::Instr_Kind::getelementptr_indirect: {
      auto const instr =
        static_cast<ir::Instr_getelementptr_indirect const*>(generic_instr);
      anton::String const referrers_str =
        stringify_referrers(ctx.allocator, instr->referrers);
      stream.write(anton::format(
        ctx.allocator,
        "%{} = getelementptr_indirect %{} %{} [referrers {}:{}] @ {}:{}\n"_sv,
        instr->id, instr->variable->id, instr->offset->id,
        instr->referrers.size(), referrers_str, instr->source_info.line,
        instr->source_info.column));
    } break;

    case ir::Instr_Kind::store: {
      auto const instr = static_cast<ir::Instr_store const*>(generic_instr);
      stream.write(anton::format(
        ctx.allocator, "store %{}, %{} @ {}:{}\n"_sv, instr->dst->id,
        instr->src->id, instr->source_info.line, instr->source_info.column));
    } break;

    case ir::Instr_Kind::store_indirect: {
      auto const instr =
        static_cast<ir::Instr_store_indirect const*>(generic_instr);
      stream.write(anton::format(
        ctx.allocator, "store_indirect %{}, %{} @ {}:{}\n"_sv, instr->dst->id,
        instr->src->id, instr->source_info.line, instr->source_info.column));
    } break;

    case ir::Instr_Kind::load: {
      auto const instr = static_cast<ir::Instr_load const*>(generic_instr);
      anton::String const referrers_str =
        stringify_referrers(ctx.allocator, instr->referrers);
      stream.write(anton::format(
        ctx.allocator, "%{} = load %{} [referrers {}:{}] @ {}:{}\n"_sv,
        instr->id, instr->src->id, instr->referrers.size(), referrers_str,
        instr->source_info.line, instr->source_info.column));
    } break;

    case ir::Instr_Kind::load_indirect: {
      auto const instr =
        static_cast<ir::Instr_load_indirect const*>(generic_instr);
      anton::String const referrers_str =
        stringify_referrers(ctx.allocator, instr->referrers);
      stream.write(anton::format(
        ctx.allocator, "%{} = load_indirect %{} [referrers {}:{}] @ {}:{}\n"_sv,
        instr->id, instr->src->id, instr->referrers.size(), referrers_str,
        instr->source_info.line, instr->source_info.column));
    } break;

    case ir::Instr_Kind::load_rhs: {
      auto const instr = static_cast<ir::Instr_load_rhs const*>(generic_instr);
      anton::String const referrers_str =
        stringify_referrers(ctx.allocator, instr->referrers);
      stream.write(anton::format(
        ctx.allocator, "%{} = load_rhs %{} [referrers {}:{}] @ {}:{}\n"_sv,
        instr->id, instr->src->id, instr->referrers.size(), referrers_str,
        instr->source_info.line, instr->source_info.column));
    } break;

    case ir::Instr_Kind::alu: {
      auto const instr = static_cast<ir::Instr_ALU const*>(generic_instr);
      anton::String_View const opcode = stringify(instr->opcode);
      anton::String const referrers_str =
        stringify_referrers(ctx.allocator, instr->referrers);
      stream.write(anton::format(
        ctx.allocator, "%{} = alu {} %{}, %{} [referrers {}:{}] @ {}:{}\n"_sv,
        instr->id, opcode, instr->lhs->id, instr->rhs->id,
        instr->referrers.size(), referrers_str, instr->source_info.line,
        instr->source_info.column));
    } break;

    case ir::Instr_Kind::end: {
      stream.write(anton::format(ctx.allocator, "end @ {}:{}\n"_sv,
                                 generic_instr->source_info.line,
                                 generic_instr->source_info.column));
    } break;

    case ir::Instr_Kind::ret: {
      stream.write(anton::format(ctx.allocator, "ret @ {}:{}\n"_sv,
                                 generic_instr->source_info.line,
                                 generic_instr->source_info.column));
    } break;

    case ir::Instr_Kind::branch: {
      auto const instr = static_cast<ir::Instr_branch const*>(generic_instr);
      stream.write(anton::format(ctx.allocator, "branch _B{} @ {}:{}\n"_sv,
                                 instr->target->label, instr->source_info.line,
                                 instr->source_info.column));
      print_block(ctx, stream, instr->target, indent - 2);
    } break;

    case ir::Instr_Kind::brcond: {
      auto const instr = static_cast<ir::Instr_brcond const*>(generic_instr);
      anton::String_View const kind = stringify(instr->kind);
      stream.write(
        anton::format(ctx.allocator, "brcond {} %{}, _B{}, _B{} @ {}:{}\n"_sv,
                      kind, instr->condition->id, instr->then_target->label,
                      instr->else_target->label, instr->source_info.line,
                      instr->source_info.column));
      print_block(ctx, stream, instr->then_target, indent - 2);
      print_block(ctx, stream, instr->else_target, indent - 2);
    } break;

    case ir::Instr_Kind::call: {
      auto const instr = static_cast<ir::Instr_call const*>(generic_instr);
      stream.write(anton::format(ctx.allocator, "call {}"_sv,
                                 instr->function->identifier));
      for(ir::Instr* const arg: instr->arguments) {
        stream.write(anton::format(ctx.allocator, " %{}"_sv, arg->id));
      }
      stream.write(anton::format(ctx.allocator, " @ {}:{}\n",
                                 instr->source_info.line,
                                 instr->source_info.column));
    } break;

    case ir::Instr_Kind::read: {
      auto const instr = static_cast<ir::Instr_read const*>(generic_instr);
      stream.write(anton::format(ctx.allocator, "%{} = Instr_read @ {}:{}\n"_sv,
                                 instr->id, instr->source_info.line,
                                 instr->source_info.column));
    } break;

    case ir::Instr_Kind::write: {
      auto const instr = static_cast<ir::Instr_write const*>(generic_instr);
      stream.write(anton::format(ctx.allocator, "write %{} @ {}:{}\n"_sv,
                                 instr->src->id, instr->source_info.line,
                                 instr->source_info.column));
    } break;
    }
  }

  void print_block(PP_Context& ctx, anton::Output_Stream& stream,
                   ir::Block const* const block, i64 const indent)
  {
    if(ctx.visited_blocks.find(block->label) != ctx.visited_blocks.end()) {
      return;
    }

    ctx.visited_blocks.emplace(block->label);
    print_indent(stream, indent);
    stream.write(anton::format(ctx.allocator, "BLOCK _B{}\n"_sv, block->label));
    for(ir::Instr const* instr: *block) {
      print_instr(ctx, stream, instr, indent + 2);
    }
  }

  static void print_ir(PP_Context& ctx, anton::Output_Stream& stream,
                       ir::Function const* const function, i64 const indent)
  {
    print_indent(stream, indent);
    if(function->entry) {
      stream.write(anton::format(ctx.allocator, "ENTRY main @ {}:{}\n"_sv,
                                 function->source_info.line,
                                 function->source_info.column));
      print_indent(stream, indent + 2);
      stream.write("LOCALS\n");
      for(ir::Instr_variable const* const local: function->locals) {
        print_instr(ctx, stream, local, indent + 4);
      }
      print_block(ctx, stream, function->entry_block, indent + 2);
    } else {
      stream.write(anton::format(
        ctx.allocator, "FUNCTION {} @ {}:{}\n"_sv, function->identifier,
        function->source_info.line, function->source_info.column));
      print_indent(stream, indent + 2);
      stream.write("PARAMETERS\n");
      for(ir::Instr_variable const* const parameter: function->parameters) {
        print_instr(ctx, stream, parameter, indent + 4);
      }
      print_indent(stream, indent + 2);
      stream.write("LOCALS\n");
      for(ir::Instr_variable const* const local: function->locals) {
        print_instr(ctx, stream, local, indent + 4);
      }
      print_block(ctx, stream, function->entry_block, indent + 2);
    }
  }

  void prettyprint_ir(Allocator* allocator, anton::Output_Stream& stream,
                      ir::Function const* function)
  {
    PP_Context ctx{.allocator = allocator, .visited_blocks = {allocator}};
    print_ir(ctx, stream, function, 0);
  }
} // namespace glang
