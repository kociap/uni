#pragma once

#include <anton/array.hpp>
#include <anton/string.hpp>

#include <glang_core/ilist.hpp>
#include <glang_core/source_info.hpp>
#include <glang_core/types.hpp>
#include <glang_ir/context.hpp>
#include <glang_ir/opcodes.hpp>

namespace glang::ir {
  struct Block;
  struct Function;
  struct Module;

  enum struct Instr_Kind : u8 {
    variable,
    constant,
    getelementptr,
    getelementptr_indirect,
    store,
    store_indirect,
    load,
    load_indirect,
    load_rhs,
    alu,
    end,
    ret,
    call,
    branch,
    brcond,
    read,
    write,
  };

  struct Instr: public IList_Node<Instr> {
    anton::Array<Instr*> referrers;
    Instr_Kind instr_kind;
    Source_Info source_info;
    i64 id;

    Instr(Allocator* allocator, Instr_Kind instr_kind, i64 const id,
          Source_Info const& source_info)
      : referrers(allocator), instr_kind(instr_kind), source_info(source_info),
        id(id)
    {
    }

    void add_referrer(Instr* instruction)
    {
      referrers.push_back(instruction);
    }

    void remove_referrer(Instr* instruction)
    {
      Instr** b = referrers.begin();
      Instr** const e = referrers.end();
      for(; b != e; ++b) {
        if(*b == instruction) {
          referrers.erase_unsorted(b);
          break;
        }
      }
    }
  };

  void replace_uses_with(Instr* instruction, Instr* replacement);

  [[nodiscard]] bool is_branch(Instr const* instruction);

  enum struct Variable_Kind {
    local,
    param,
  };

  struct Instr_variable: public Instr {
    Variable_Kind kind;
    anton::String identifier;
    i64 size = 1;

    Instr_variable(Allocator* allocator, i64 id, Variable_Kind kind,
                   anton::String&& identifier, Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::variable, id, source_info), kind(kind),
        identifier(ANTON_MOV(identifier))
    {
    }
  };

  struct Instr_constant: public Instr {
    i64 value;

    Instr_constant(Allocator* allocator, i64 id, i64 value,
                   Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::constant, id, source_info), value(value)
    {
    }
  };

  struct Instr_getelementptr: public Instr {
    Instr_variable* variable;
    Instr* offset;

    Instr_getelementptr(Allocator* allocator, i64 id, Instr_variable* variable,
                        Instr* offset, Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::getelementptr, id, source_info),
        variable(variable), offset(offset)
    {
    }
  };

  struct Instr_getelementptr_indirect: public Instr {
    Instr_variable* variable;
    Instr* offset;

    Instr_getelementptr_indirect(Allocator* allocator, i64 id,
                                 Instr_variable* variable, Instr* offset,
                                 Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::getelementptr_indirect, id, source_info),
        variable(variable), offset(offset)
    {
    }
  };

  struct Instr_store: public Instr {
    Instr* dst;
    Instr* src;

    Instr_store(Allocator* allocator, i64 id, Instr* dst, Instr* src,
                Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::store, id, source_info), dst(dst), src(src)
    {
    }
  };

  struct Instr_store_indirect: public Instr {
    Instr* dst;
    Instr* src;

    Instr_store_indirect(Allocator* allocator, i64 id, Instr* dst, Instr* src,
                         Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::store_indirect, id, source_info), dst(dst),
        src(src)
    {
    }
  };

  struct Instr_load: public Instr {
    Instr* src;

    Instr_load(Allocator* allocator, i64 id, Instr* src,
               Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::load, id, source_info), src(src)
    {
    }
  };

  struct Instr_load_indirect: public Instr {
    Instr* src;

    Instr_load_indirect(Allocator* allocator, i64 id, Instr* src,
                        Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::load_indirect, id, source_info), src(src)
    {
    }
  };

  struct Instr_load_rhs: public Instr {
    Instr* src;

    Instr_load_rhs(Allocator* allocator, i64 id, Instr* src,
                   Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::load_rhs, id, source_info), src(src)
    {
    }
  };

  struct Instr_ALU: public Instr {
    Instr* lhs;
    Instr* rhs;
    ALU_Opcode opcode;

    Instr_ALU(Allocator* allocator, i64 id, ALU_Opcode opcode, Instr* lhs,
              Instr* rhs, Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::alu, id, source_info), lhs(lhs), rhs(rhs),
        opcode(opcode)
    {
    }
  };

  struct Instr_end: public Instr {
    Instr_end(Allocator* allocator, i64 id, Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::end, id, source_info)
    {
    }
  };

  struct Instr_ret: public Instr {
    Instr_ret(Allocator* allocator, i64 id, Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::ret, id, source_info)
    {
    }
  };

  struct Instr_branch: public Instr {
    Block* target;

    Instr_branch(Allocator* allocator, i64 id, Block* target,
                 Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::branch, id, source_info), target(target)
    {
    }
  };

  enum struct Brcond_Kind : u8 {
    jgt,
    jz,
  };

  struct Instr_brcond: public Instr {
    Block* then_target;
    Block* else_target;
    Instr* condition;
    Brcond_Kind kind;

    Instr_brcond(Allocator* allocator, i64 id, Brcond_Kind kind,
                 Instr* condition, Block* then_target, Block* else_target,
                 Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::brcond, id, source_info),
        then_target(then_target), else_target(else_target),
        condition(condition), kind(kind)
    {
    }
  };

  struct Instr_call: public Instr {
    Function* function;
    anton::Array<Instr*> arguments;

    Instr_call(Allocator* allocator, i64 id, Function* function,
               Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::call, id, source_info), function(function),
        arguments(allocator)
    {
    }
  };

  struct Instr_read: public Instr {
    Instr_read(Allocator* allocator, i64 id, Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::read, id, source_info)
    {
    }
  };

  struct Instr_write: public Instr {
    Instr* src;

    Instr_write(Allocator* allocator, i64 id, Instr* src,
                Source_Info const& source_info)
      : Instr(allocator, Instr_Kind::write, id, source_info), src(src)
    {
    }
  };

  [[nodiscard]] Instr_variable*
  make_instr_variable(Context& ctx, Source_Info const& source_info,
                      Variable_Kind kind, anton::String_View identifier);
  [[nodiscard]] Instr_constant*
  make_instr_constant(Context& ctx, Source_Info const& source_info, i64 value);

  [[nodiscard]] Instr_getelementptr*
  make_instr_getelementptr(Context& ctx, Source_Info const& source_info,
                           Instr_variable* variable, Instr* offset);
  [[nodiscard]] Instr_getelementptr_indirect*
  make_instr_getelementptr_indirect(Context& ctx,
                                    Source_Info const& source_info,
                                    Instr_variable* variable, Instr* offset);

  [[nodiscard]] Instr_store* make_instr_store(Context& ctx,
                                              Source_Info const& source_info,
                                              Instr* dst, Instr* src);

  [[nodiscard]] Instr_store_indirect*
  make_instr_store_indirect(Context& ctx, Source_Info const& source_info,
                            Instr* dst, Instr* src);

  [[nodiscard]] Instr_load*
  make_instr_load(Context& ctx, Source_Info const& source_info, Instr* src);

  [[nodiscard]] Instr_load_indirect*
  make_instr_load_indirect(Context& ctx, Source_Info const& source_info,
                           Instr* src);

  [[nodiscard]] Instr_load_rhs*
  make_instr_load_rhs(Context& ctx, Source_Info const& source_info, Instr* src);

  [[nodiscard]] Instr_ALU* make_instr_alu(Context& ctx,
                                          Source_Info const& source_info,
                                          ALU_Opcode opcode, Instr* lhs,
                                          Instr* rhs);

  [[nodiscard]] Instr_end* make_instr_end(Context& ctx,
                                          Source_Info const& source_info);

  [[nodiscard]] Instr_ret* make_instr_ret(Context& ctx,
                                          Source_Info const& source_info);

  [[nodiscard]] Instr_branch* make_instr_branch(Context& ctx,
                                                Source_Info const& source_info,
                                                Block* target);

  [[nodiscard]] Instr_brcond*
  make_instr_brcond(Context& ctx, Source_Info const& source_info,
                    Brcond_Kind kind, Instr* condition, Block* then_target,
                    Block* else_target);

  [[nodiscard]] Instr_call* make_instr_call(Context& ctx,
                                            Source_Info const& source_info,
                                            Function* function);

  [[nodiscard]] Instr_read* make_instr_read(Context& ctx,
                                            Source_Info const& source_info);

  [[nodiscard]] Instr_write*
  make_instr_write(Context& ctx, Source_Info const& source_info, Instr* src);

  struct Block {
  public:
    using iterator = IList<Instr>::iterator;
    using const_iterator = IList<Instr>::const_iterator;

  private:
    IList<Instr> instructions;

  public:
    i64 label;
    Function* function;

    Block(Function* function, i64 label): label(label), function(function) {}

    void insert(Instr* instruction)
    {
      instructions.insert_back(instruction);
    }

    [[nodiscard]] Instr* get_first()
    {
      return *instructions.begin();
    }

    [[nodiscard]] Instr* get_last()
    {
      return *(--instructions.end());
    }

    [[nodiscard]] iterator begin()
    {
      return instructions.begin();
    }

    [[nodiscard]] const_iterator begin() const
    {
      return instructions.begin();
    }

    [[nodiscard]] iterator end()
    {
      return instructions.end();
    }

    [[nodiscard]] const_iterator end() const
    {
      return instructions.end();
    }
  };

  [[nodiscard]] ir::Block* make_block(Context& ctx, Function* function);

  // split_block
  //
  // Split the block into two blocks at the specified instruction. An
  // unconditional branch is added to the original block and the remaining
  // instructions are moved to the new block.
  //
  [[nodiscard]] Block* split_block(Context& ctx, Function* function, i64 label,
                                   Instr* instruction, Instr* end,
                                   bool before = false);

  struct Function: public IList_Node<Function> {
    anton::String identifier;
    anton::Array<Instr_variable*> parameters;
    anton::Array<Instr_variable*> locals;
    anton::Array<Instr_call*> referrers;
    Block* entry_block = nullptr;
    bool entry = false;
    Source_Info source_info;

    Function(Allocator* allocator, anton::String&& identifier,
             Source_Info const& source_info)
      : identifier(ANTON_MOV(identifier)), parameters(allocator),
        locals(allocator), referrers(allocator), source_info(source_info)
    {
    }

    void add_referrer(Instr_call* instruction)
    {
      referrers.push_back(instruction);
    }

    void add_local(Instr_variable* variable)
    {
      locals.push_back(variable);
    }
  };
} // namespace glang::ir
