#pragma once

#include <anton/stream.hpp>

#include <glang_core/types.hpp>
#include <glang_ir/ir.hpp>

namespace glang {
  void prettyprint_ir(Allocator* allocator, anton::Output_Stream& stream,
                      ir::Function const* function);
}
