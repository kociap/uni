#pragma once

#include <anton/stream.hpp>

#include <glang_syntax/syntax.hpp>

namespace glang {
  void prettyprint_syntax_tree(Allocator* allocator,
                               anton::Output_Stream& stream, SNOT const& snot);
}
