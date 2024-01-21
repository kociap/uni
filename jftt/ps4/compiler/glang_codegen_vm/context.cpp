#include <glang_codegen_vm/context.hpp>

#include <glang_codegen_vm/lir.hpp>
#include <glang_core/memory.hpp>

namespace glang::lir {
  i64 Context::get_id()
  {
    return ++bound;
  }

  i64 Context::allocate_addresses(i64 count)
  {
    i64 const offset = current_address;
    current_address += count;
    return offset;
  }

  Variable* Context::allocate_variable(i64 const size, bool const pointer)
  {
    i64 const address = allocate_addresses(size);
    i64 const id = get_id();
    Variable* const variable =
      allocate<Variable>(allocator, id, address, pointer);
    return variable;
  }
} // namespace glang::lir
