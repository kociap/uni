#pragma once

#include <optional>
#include <span>
#include <vector>

#include <heuristic.hpp>
#include <types.hpp>

namespace puzzle15 {
  [[nodiscard]] bool is_solvable(Configuration_View configuration);
}

template<typename Configuration>
struct Solution {
  std::vector<Configuration> path;
  i64 iterations = 0;
  i64 depth = 0;
  i64 explored = 0;
  bool found = false;
};

namespace puzzle15 {
  using Solution = Solution<Configuration>;
}

struct IDAstar_Parameters {
  Configuration_View starting_configuration;
  heuristic_t heuristic;
  i32 initial_f_cutoff = 0;
  // Maximum number of deepenings iterations. 0 means unlimited.
  i32 max_iterations = 0;
};

namespace puzzle15 {
  // Empty square is the one with the largest value.
  [[nodiscard]] Solution IDAstar_solver(IDAstar_Parameters parameters);
} // namespace puzzle15

struct Astar_Parameters {
  Configuration_View starting_configuration;
  heuristic_t heuristic;
};

namespace puzzle15 {
  // Empty square is the one with the largest value.
  [[nodiscard]] Solution Astar_solver(Astar_Parameters parameters);
} // namespace puzzle15

struct Bidirectional_Astar_Parameters {
  Configuration_View starting_configuration;
  heuristic_t forward_heuristic;
  generic_heuristic_t backward_heuristic;
};

namespace puzzle15 {
  // Empty square is denoted by 16.
  [[nodiscard]] Solution
  Bidirectional_Astar_solver(Bidirectional_Astar_Parameters parameters);
} // namespace puzzle15