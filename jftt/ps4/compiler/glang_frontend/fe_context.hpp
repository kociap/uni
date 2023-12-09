#pragma once

#include <glang_core/types.hpp>

namespace glang {
  struct Options {
    bool enable_case_sensitive_keywords = false;
    bool extended_diagnostics = true;
  };

  struct FE_Context {
    Allocator* allocator;
    Options options;
  };
} // namespace glang
