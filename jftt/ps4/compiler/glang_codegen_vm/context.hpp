#pragma once

#include <anton/flat_hash_map.hpp>
#include <anton/flat_hash_set.hpp>

#include <glang_core/types.hpp>

namespace glang::ir {
  struct Block;
  struct Function;
} // namespace glang::ir

namespace glang::lir {
  struct Instr;
  struct Variable;

  struct Block_Data {
    Instr* first_instr;
  };

  struct Function_Data {
    Instr* first_instr;
    Variable* return_address;
  };

  struct Context {
  public:
    anton::Flat_Hash_Map<ir::Block const*, Block_Data> blocks;
    anton::Flat_Hash_Map<ir::Function const*, Function_Data> functions;
    anton::Flat_Hash_Map<i64, Variable*> variables;
    Allocator* allocator;
    i64 current_address = 0;
    i64 bound = 0;

    [[nodiscard]] i64 get_id();

    [[nodiscard]] i64 allocate_addresses(i64 count);

    [[nodiscard]] Variable* allocate_variable(i64 size, bool pointer);
  };
} // namespace glang::lir
