#pragma once

#include <anton/array.hpp>

#include <glang_codegen_vm/context.hpp>
#include <glang_core/ilist.hpp>
#include <glang_core/source_info.hpp>

namespace glang::ir {
  struct Block;
}

namespace glang::lir {
  // Calling convention is return address in register a.

  enum struct Instr_Kind {
    // read
    // read to reg a.
    read,
    // write
    // write from reg a.
    write,
    // load x
    // reg a = mem[reg x]
    load,
    // store x
    // mem[reg x] = reg a
    store,
    // add x
    // reg a = reg a + reg x
    add,
    // sub x
    // reg a = reg a - reg x
    sub,
    // get x
    // reg a = reg x
    get,
    // put x
    // reg x = reg a
    put,
    // rst x
    // reg x = 0
    rst,
    // inc x
    // reg x = reg x + 1
    inc,
    // dec x
    // reg x = reg x - 1
    dec,
    // shl x
    // reg x = reg x * 2
    shl,
    // shr x
    // reg x = reg x / 2
    shr,
    // jump j
    // Set program counter to j.
    jump,
    // jpos j
    // Conditionally set program counter to j if reg a > 0.
    jpos,
    // jzero j
    // Conditionally set program counter to j if reg a == 0.
    jzero,
    // strk x
    // Store the program counter to reg x.
    strk,
    // jumpr x
    // Set the program counter to reg x.
    jumpr,
    // halt
    // End execution of the program.
    halt,
  };

  enum struct Reg_Kind {
    a,
    b,
    c,
    d,
    e,
    f,
    g,
    h,
  };

  struct Reg {
    Reg_Kind kind;

    Reg(Reg_Kind kind): kind(kind) {}
  };

  [[nodiscard]] Reg* make_reg(Context& ctx, Reg_Kind kind);

  struct Instr: IList_Node<Instr> {
    anton::Array<Instr*> incoming;
    Instr_Kind instr_kind;
    Source_Info source_info;

    Instr(Instr_Kind instr_kind, Source_Info const& source_info)
      : instr_kind(instr_kind), source_info(source_info)
    {
    }

    void add_incoming(Instr* instruction)
    {
      incoming.push_back(instruction);
    }

    void remove_incoming(Instr* instruction)
    {
      Instr** b = incoming.begin();
      Instr** const e = incoming.end();
      for(; b != e; ++b) {
        if(*b == instruction) {
          incoming.erase_unsorted(b);
          break;
        }
      }
    }
  };

  struct Variable {
    i64 id;
    i64 address;
    bool pointer;
  };

  // struct Instr_call: public Instr {
  //   Instr_call(Source_Info const& source_info)
  //     : Instr(Instr_Kind::call, source_info)
  //   {
  //   }
  // };

  struct Instr_read: public Instr {
    Instr_read(Source_Info const& source_info)
      : Instr(Instr_Kind::read, source_info)
    {
    }
  };

  struct Instr_write: public Instr {
    Instr_write(Source_Info const& source_info)
      : Instr(Instr_Kind::write, source_info)
    {
    }
  };

  struct Instr_load: public Instr {
    Reg* location;

    Instr_load(Reg* location, Source_Info const& source_info)
      : Instr(Instr_Kind::load, source_info), location(location)
    {
    }
  };

  struct Instr_store: public Instr {
    Reg* location;

    Instr_store(Reg* location, Source_Info const& source_info)
      : Instr(Instr_Kind::store, source_info), location(location)
    {
    }
  };

  struct Instr_add: public Instr {
    Reg* operand;

    Instr_add(Reg* operand, Source_Info const& source_info)
      : Instr(Instr_Kind::add, source_info), operand(operand)
    {
    }
  };

  struct Instr_sub: public Instr {
    Reg* operand;

    Instr_sub(Reg* operand, Source_Info const& source_info)
      : Instr(Instr_Kind::sub, source_info), operand(operand)
    {
    }
  };

  struct Instr_get: public Instr {
    Reg* src;

    Instr_get(Reg* src, Source_Info const& source_info)
      : Instr(Instr_Kind::get, source_info), src(src)
    {
    }
  };

  struct Instr_put: public Instr {
    Reg* dst;

    Instr_put(Reg* dst, Source_Info const& source_info)
      : Instr(Instr_Kind::put, source_info), dst(dst)
    {
    }
  };

  struct Instr_rst: public Instr {
    Reg* operand;

    Instr_rst(Reg* operand, Source_Info const& source_info)
      : Instr(Instr_Kind::rst, source_info), operand(operand)
    {
    }
  };

  struct Instr_inc: public Instr {
    Reg* operand;

    Instr_inc(Reg* operand, Source_Info const& source_info)
      : Instr(Instr_Kind::inc, source_info), operand(operand)
    {
    }
  };

  struct Instr_dec: public Instr {
    Reg* operand;

    Instr_dec(Reg* operand, Source_Info const& source_info)
      : Instr(Instr_Kind::dec, source_info), operand(operand)
    {
    }
  };

  struct Instr_shl: public Instr {
    Reg* operand;

    Instr_shl(Reg* operand, Source_Info const& source_info)
      : Instr(Instr_Kind::shl, source_info), operand(operand)
    {
    }
  };

  struct Instr_shr: public Instr {
    Reg* operand;

    Instr_shr(Reg* operand, Source_Info const& source_info)
      : Instr(Instr_Kind::shr, source_info), operand(operand)
    {
    }
  };

  struct Instr_jump: public Instr {
    Instr* target;
    i64 jump_ip;

    Instr_jump(Instr* target, Source_Info const& source_info)
      : Instr(Instr_Kind::jump, source_info), target(target)
    {
    }
  };

  struct Instr_jpos: public Instr {
    Instr* target;
    i64 jump_ip;

    Instr_jpos(Instr* target, Source_Info const& source_info)
      : Instr(Instr_Kind::jpos, source_info), target(target)
    {
    }
  };

  struct Instr_jzero: public Instr {
    Instr* target;
    i64 jump_ip;

    Instr_jzero(Instr* target, Source_Info const& source_info)
      : Instr(Instr_Kind::jzero, source_info), target(target)
    {
    }
  };

  struct Instr_strk: public Instr {
    Instr_strk(Source_Info const& source_info)
      : Instr(Instr_Kind::strk, source_info)
    {
    }
  };

  struct Instr_jumpr: public Instr {
    Reg* address;

    Instr_jumpr(Reg* address, Source_Info const& source_info)
      : Instr(Instr_Kind::jumpr, source_info), address(address)
    {
    }
  };

  struct Instr_halt: public Instr {
    Instr_halt(Source_Info const& source_info)
      : Instr(Instr_Kind::halt, source_info)
    {
    }
  };

  // [[nodiscard]] Instr_call* make_instr_call(Context& ctx,
  //                                           Source_Info const& source_info);
  [[nodiscard]] Instr_read* make_instr_read(Context& ctx,
                                            Source_Info const& source_info);
  [[nodiscard]] Instr_write* make_instr_write(Context& ctx,
                                              Source_Info const& source_info);
  [[nodiscard]] Instr_load* make_instr_load(Context& ctx, Reg* location,
                                            Source_Info const& source_info);
  [[nodiscard]] Instr_store* make_instr_store(Context& ctx, Reg* location,
                                              Source_Info const& source_info);
  [[nodiscard]] Instr_add* make_instr_add(Context& ctx, Reg* operand,
                                          Source_Info const& source_info);
  [[nodiscard]] Instr_sub* make_instr_sub(Context& ctx, Reg* operand,
                                          Source_Info const& source_info);
  [[nodiscard]] Instr_get* make_instr_get(Context& ctx, Reg* src,
                                          Source_Info const& source_info);
  [[nodiscard]] Instr_put* make_instr_put(Context& ctx, Reg* dst,
                                          Source_Info const& source_info);
  [[nodiscard]] Instr_rst* make_instr_rst(Context& ctx, Reg* operand,
                                          Source_Info const& source_info);
  [[nodiscard]] Instr_inc* make_instr_inc(Context& ctx, Reg* operand,
                                          Source_Info const& source_info);
  [[nodiscard]] Instr_dec* make_instr_dec(Context& ctx, Reg* operand,
                                          Source_Info const& source_info);
  [[nodiscard]] Instr_shl* make_instr_shl(Context& ctx, Reg* operand,
                                          Source_Info const& source_info);
  [[nodiscard]] Instr_shr* make_instr_shr(Context& ctx, Reg* operand,
                                          Source_Info const& source_info);
  [[nodiscard]] Instr_jump* make_instr_jump(Context& ctx, Instr* target,
                                            Source_Info const& source_info);
  [[nodiscard]] Instr_jpos* make_instr_jpos(Context& ctx, Instr* target,
                                            Source_Info const& source_info);
  [[nodiscard]] Instr_jzero* make_instr_jzero(Context& ctx, Instr* target,
                                              Source_Info const& source_info);
  [[nodiscard]] Instr_strk* make_instr_strk(Context& ctx,
                                            Source_Info const& source_info);
  [[nodiscard]] Instr_jumpr* make_instr_jumpr(Context& ctx, Reg* address,
                                              Source_Info const& source_info);
  [[nodiscard]] Instr_halt* make_instr_halt(Context& ctx,
                                            Source_Info const& source_info);
} // namespace glang::lir
