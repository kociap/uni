#pragma once

#include <types.hpp>

using heuristic_t = i32 (*)(Configuration_View configuration);
using generic_heuristic_t = i32 (*)(Configuration_View start,
                                    Configuration_View goal);

namespace puzzle15 {
  // heuristic_manhattan_distance
  // Optimised version of the Manhattan Distance (MD) for calculating the MD
  // to the standard goal configuration.
  //
  [[nodiscard]] i32 heuristic_manhattan_distance(Configuration_View start);

  // heuristic_manhattan_distance_generic
  // Generic version of the Manhattan Distance (MD) for calculating the MD
  // to any goal start.
  //
  [[nodiscard]] i32
  heuristic_manhattan_distance_generic(Configuration_View start,
                                       Configuration_View goal);

  [[nodiscard]] i32 heuristic_linear_conflict(Configuration_View start);

  [[nodiscard]] i32 heuristic_linear_conflict_generic(Configuration_View start,
                                                      Configuration_View goal);

  [[nodiscard]] i32 heuristic_inversions(Configuration_View start);

  void build_pattern_database();

  [[nodiscard]] i32 heuristic_pattern_database(Configuration_View start);
  [[nodiscard]] i32 heuristic_pattern_database_generic(Configuration_View start,
                                                       Configuration_View goal);
} // namespace puzzle15