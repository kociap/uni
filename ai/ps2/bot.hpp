#pragma once

#include <configuration.hpp>
#include <types.hpp>

struct MINMAX_Result {
  i32 value;
  // The row of the best move found.
  i32 x;
  // The column of the best move found.
  i32 y;
};

struct MINMAX_Parameters {
  Configuration c;
  Player player;
  Player turn;
  i32 depth;
  i32 max_depth;
  i32 alpha;
  i32 beta;
};

[[nodiscard]] MINMAX_Result minmax_search(MINMAX_Parameters p);

