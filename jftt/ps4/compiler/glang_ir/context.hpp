#pragma once

#include <glang_core/types.hpp>

namespace glang::ir {
  struct Context {
  public:
    Allocator* allocator;

  private:
    i64 id = 0;
    i64 block_label = 0;

  public:
    Context(Allocator* allocator): allocator(allocator) {}

    [[nodiscard]] i64 get_next_id()
    {
      i64 const value = id;
      id += 1;
      return value;
    }

    [[nodiscard]] i64 get_next_block_label()
    {
      i64 const value = block_label;
      block_label += 1;
      return value;
    }
  };
} // namespace glang::ir
