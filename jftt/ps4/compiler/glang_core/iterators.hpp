#pragma once

#include <anton/iterators/range.hpp>
#include <anton/iterators/zip.hpp>

namespace glang {
  template<typename... Rangelike>
  auto zip(Rangelike&... rangelike)
  {
    return anton::Range(anton::Zip_Iterator(rangelike.begin()...),
                        anton::Zip_Iterator(rangelike.end()...));
  }
} // namespace glang
