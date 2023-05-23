#pragma once

#include <configuration.hpp>
#include <types.hpp>

// heuristic
// Calculate favourability of a position for player.
//
[[nodiscard]] i32 heuristic(Configuration const& c, Player player);
