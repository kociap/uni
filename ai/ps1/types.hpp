#pragma once

#include <array>
#include <span>
#include <stdio.h>

using i8 = char;
using u8 = unsigned char;
using i32 = int;
using u32 = unsigned int;
using i64 = long long;
using u64 = unsigned long long;

constexpr u32 u32_largest_value = 0xFFFFFFFF;
constexpr i32 i32_largest_value = 0x7FFFFFFF;

using Configuration_Entry = i8;
using Configuration_View = std::span<Configuration_Entry const>;

namespace puzzle15 {
  using Configuration = std::array<Configuration_Entry, 16>;

  [[nodiscard]] constexpr Configuration
  copy_configuration(Configuration_View const view) {
    // configuration is ALWAYS initialised, but the compiler keeps bitching
    // about it being maybe uninitialised, hence we do the stupid and initialise
    // the configuration to 0s and only then copy to it.
    Configuration configuration = {};
    std::copy(view.begin(), view.end(), configuration.begin());
    return configuration;
  }

  [[nodiscard]] constexpr u64
  hash_configuration(Configuration_View const view) {
    u64 result = 0;
    for(i8 v: view) {
      result = (result << 4) | (v & 0x0F);
    }
    return result;
  }

  inline void print(Configuration_View const configuration) {
    for(i32 n = 0; Configuration_Entry const v: configuration) {
      if(n > 0 && (n % 4) == 0) {
        printf("\n");
      }
      if(v != 16) {
        printf(" %2d", v);
      } else {
        printf(" \033[38;5;9m%2d\033[0m", v);
      }
      n += 1;
    }
    printf("\n");
  }
} // namespace puzzle15
