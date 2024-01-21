#pragma once

#include <anton/stream.hpp>

#include <glang_codegen_vm/lir.hpp>
#include <glang_core/types.hpp>

namespace glang {
  void prettyprint_lir(Allocator* allocator, anton::Output_Stream& stream,
                       IList<lir::Instr>& instructions);
}
