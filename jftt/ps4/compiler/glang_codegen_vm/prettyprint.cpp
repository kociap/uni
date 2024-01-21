#include <glang_codegen_vm/prettyprint.hpp>

#include <anton/flat_hash_set.hpp>
#include <anton/format.hpp>
#include <anton/string_view.hpp>

namespace glang {
  using namespace anton::literals;

  struct PP_Context {
    Allocator* allocator;
    i64 ip;
  };

  static void print_indent(anton::Output_Stream& stream, i64 const indent)
  {
    for(i64 i = 0; i < indent; i += 1) {
      stream.write(" "_sv);
    }
  }

  [[nodiscard]] anton::String_View stringify(lir::Reg_Kind kind)
  {
    switch(kind) {
    case lir::Reg_Kind::a:
      return "a"_sv;
    case lir::Reg_Kind::b:
      return "b"_sv;
    case lir::Reg_Kind::c:
      return "c"_sv;
    case lir::Reg_Kind::d:
      return "d"_sv;
    case lir::Reg_Kind::e:
      return "e"_sv;
    case lir::Reg_Kind::f:
      return "f"_sv;
    case lir::Reg_Kind::g:
      return "g"_sv;
    case lir::Reg_Kind::h:
      return "h"_sv;
    }
  }

  static void print_instr(PP_Context& ctx, anton::Output_Stream& stream,
                          lir::Instr const* const generic_instr,
                          i64 const indent)
  {
    print_indent(stream, indent);
    // stream.write(anton::format(ctx.allocator, "{} "_sv, ctx.ip));
    bool const print_source_info = false;
    switch(generic_instr->instr_kind) {
    case lir::Instr_Kind::read: {
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "READ @ {}:{}\n"_sv,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "READ\n"_sv));
      }
    } break;

    case lir::Instr_Kind::write: {
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "WRITE @ {}:{}\n"_sv,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "WRITE\n"_sv));
      }
    } break;

    case lir::Instr_Kind::load: {
      auto const instr = static_cast<lir::Instr_load const*>(generic_instr);
      anton::String_View const reg = stringify(instr->location->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "LOAD {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "LOAD {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::store: {
      auto const instr = static_cast<lir::Instr_store const*>(generic_instr);
      anton::String_View const reg = stringify(instr->location->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "STORE {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "STORE {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::add: {
      auto const instr = static_cast<lir::Instr_add const*>(generic_instr);
      anton::String_View const reg = stringify(instr->operand->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "ADD {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "ADD {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::sub: {
      auto const instr = static_cast<lir::Instr_sub const*>(generic_instr);
      anton::String_View const reg = stringify(instr->operand->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "SUB {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "SUB {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::get: {
      auto const instr = static_cast<lir::Instr_get const*>(generic_instr);
      anton::String_View const reg = stringify(instr->src->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "GET {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "GET {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::put: {
      auto const instr = static_cast<lir::Instr_put const*>(generic_instr);
      anton::String_View const reg = stringify(instr->dst->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "PUT {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "PUT {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::rst: {
      auto const instr = static_cast<lir::Instr_rst const*>(generic_instr);
      anton::String_View const reg = stringify(instr->operand->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "RST {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "RST {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::inc: {
      auto const instr = static_cast<lir::Instr_inc const*>(generic_instr);
      anton::String_View const reg = stringify(instr->operand->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "INC {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "INC {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::dec: {
      auto const instr = static_cast<lir::Instr_dec const*>(generic_instr);
      anton::String_View const reg = stringify(instr->operand->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "DEC {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "DEC {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::shl: {
      auto const instr = static_cast<lir::Instr_shl const*>(generic_instr);
      anton::String_View const reg = stringify(instr->operand->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "SHL {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "SHL {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::shr: {
      auto const instr = static_cast<lir::Instr_shr const*>(generic_instr);
      anton::String_View const reg = stringify(instr->operand->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "SHR {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "SHR {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::jump: {
      auto const instr = static_cast<lir::Instr_jump const*>(generic_instr);
      if(print_source_info) {
        stream.write(anton::format(
          ctx.allocator, "JUMP {} @ {}:{}\n"_sv, instr->jump_ip,
          generic_instr->source_info.line, generic_instr->source_info.column));
      } else {
        stream.write(
          anton::format(ctx.allocator, "JUMP {}\n"_sv, instr->jump_ip));
      }
    } break;

    case lir::Instr_Kind::jpos: {
      auto const instr = static_cast<lir::Instr_jpos const*>(generic_instr);
      if(print_source_info) {
        stream.write(anton::format(
          ctx.allocator, "JPOS {} @ {}:{}\n"_sv, instr->jump_ip,
          generic_instr->source_info.line, generic_instr->source_info.column));
      } else {
        stream.write(
          anton::format(ctx.allocator, "JPOS {}\n"_sv, instr->jump_ip));
      }
    } break;

    case lir::Instr_Kind::jzero: {
      auto const instr = static_cast<lir::Instr_jzero const*>(generic_instr);
      if(print_source_info) {
        stream.write(anton::format(
          ctx.allocator, "JZERO {} @ {}:{}\n"_sv, instr->jump_ip,
          generic_instr->source_info.line, generic_instr->source_info.column));
      } else {
        stream.write(
          anton::format(ctx.allocator, "JZERO {}\n"_sv, instr->jump_ip));
      }
    } break;

    case lir::Instr_Kind::strk: {
      auto const instr = static_cast<lir::Instr_strk const*>(generic_instr);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "STRK a @ {}:{}\n"_sv,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "STRK a\n"_sv));
      }
    } break;

    case lir::Instr_Kind::jumpr: {
      auto const instr = static_cast<lir::Instr_jumpr const*>(generic_instr);
      anton::String_View const reg = stringify(instr->address->kind);
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "JUMPR {} @ {}:{}\n"_sv, reg,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "JUMPR {}\n"_sv, reg));
      }
    } break;

    case lir::Instr_Kind::halt: {
      if(print_source_info) {
        stream.write(anton::format(ctx.allocator, "HALT @ {}:{}\n"_sv,
                                   generic_instr->source_info.line,
                                   generic_instr->source_info.column));
      } else {
        stream.write(anton::format(ctx.allocator, "HALT\n"_sv));
      }
    } break;
    }
  }

  static void print_lir(PP_Context& ctx, anton::Output_Stream& stream,
                        IList<lir::Instr>& instructions, i64 const indent)
  {
    for(lir::Instr* instr: instructions) {
      print_instr(ctx, stream, instr, indent);
      ctx.ip += 1;
    }
  }

  void prettyprint_lir(Allocator* allocator, anton::Output_Stream& stream,
                       IList<lir::Instr>& instructions)
  {
    PP_Context ctx{.allocator = allocator, .ip = 0};
    print_lir(ctx, stream, instructions, 0);
  }
} // namespace glang
