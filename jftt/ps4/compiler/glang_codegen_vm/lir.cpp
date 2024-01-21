#include <glang_codegen_vm/lir.hpp>

#include <glang_core/memory.hpp>

namespace glang::lir {
  Reg* make_reg(Context& ctx, Reg_Kind kind)
  {
    auto const reg = allocate<Reg>(ctx.allocator, kind);
    return reg;
  }

  // Instr_call* make_instr_call(Context& ctx, Source_Info const& source_info)
  // {
  //   auto const instr = allocate<Instr_call>(ctx.allocator, source_info);
  //   return instr;
  // }

  Instr_read* make_instr_read(Context& ctx, Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_read>(ctx.allocator, source_info);
    return instr;
  }

  Instr_write* make_instr_write(Context& ctx, Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_write>(ctx.allocator, source_info);
    return instr;
  }

  Instr_load* make_instr_load(Context& ctx, Reg* location,
                              Source_Info const& source_info)
  {
    auto const instr =
      allocate<Instr_load>(ctx.allocator, location, source_info);
    return instr;
  }

  Instr_store* make_instr_store(Context& ctx, Reg* location,
                                Source_Info const& source_info)
  {
    auto const instr =
      allocate<Instr_store>(ctx.allocator, location, source_info);
    return instr;
  }

  Instr_add* make_instr_add(Context& ctx, Reg* operand,
                            Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_add>(ctx.allocator, operand, source_info);
    return instr;
  }

  Instr_sub* make_instr_sub(Context& ctx, Reg* operand,
                            Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_sub>(ctx.allocator, operand, source_info);
    return instr;
  }

  Instr_get* make_instr_get(Context& ctx, Reg* src,
                            Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_get>(ctx.allocator, src, source_info);
    return instr;
  }

  Instr_put* make_instr_put(Context& ctx, Reg* dst,
                            Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_put>(ctx.allocator, dst, source_info);
    return instr;
  }

  Instr_rst* make_instr_rst(Context& ctx, Reg* operand,
                            Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_rst>(ctx.allocator, operand, source_info);
    return instr;
  }

  Instr_inc* make_instr_inc(Context& ctx, Reg* operand,
                            Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_inc>(ctx.allocator, operand, source_info);
    return instr;
  }

  Instr_dec* make_instr_dec(Context& ctx, Reg* operand,
                            Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_dec>(ctx.allocator, operand, source_info);
    return instr;
  }

  Instr_shl* make_instr_shl(Context& ctx, Reg* operand,
                            Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_shl>(ctx.allocator, operand, source_info);
    return instr;
  }

  Instr_shr* make_instr_shr(Context& ctx, Reg* operand,
                            Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_shr>(ctx.allocator, operand, source_info);
    return instr;
  }

  Instr_jump* make_instr_jump(Context& ctx, Instr* target,
                              Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_jump>(ctx.allocator, target, source_info);
    return instr;
  }

  Instr_jpos* make_instr_jpos(Context& ctx, Instr* target,
                              Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_jpos>(ctx.allocator, target, source_info);
    return instr;
  }

  Instr_jzero* make_instr_jzero(Context& ctx, Instr* target,
                                Source_Info const& source_info)
  {
    auto const instr =
      allocate<Instr_jzero>(ctx.allocator, target, source_info);
    return instr;
  }

  Instr_strk* make_instr_strk(Context& ctx, Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_strk>(ctx.allocator, source_info);
    return instr;
  }

  Instr_jumpr* make_instr_jumpr(Context& ctx, Reg* address,
                                Source_Info const& source_info)
  {
    auto const instr =
      allocate<Instr_jumpr>(ctx.allocator, address, source_info);
    return instr;
  }

  Instr_halt* make_instr_halt(Context& ctx, Source_Info const& source_info)
  {
    auto const instr = allocate<Instr_halt>(ctx.allocator, source_info);
    return instr;
  }
} // namespace glang::lir
