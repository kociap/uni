#pragma once

#include <anton/memory/core.hpp>

#include <glang_core/types.hpp>

namespace glang {
  template<typename T, typename... Args>
  T* allocate(Allocator* allocator, Args&&... args)
  {
    T* const p = static_cast<T*>(allocator->allocate(sizeof(T), alignof(T)));
    anton::construct(p, ANTON_FWD(args)...);
    return p;
  }
} // namespace glang
