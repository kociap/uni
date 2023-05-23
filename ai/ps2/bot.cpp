#include <bot.hpp>

#include <array>
#include <optional>
#include <stdio.h>

#include <configuration.hpp>
#include <heuristic.hpp>

// Spiral starting at the center unfolding right.
static constexpr Point board_order[] = {
  // Center.
  {2, 2},
  // Midpoints of the inner square.
  {2, 1},
  {3, 2},
  {2, 3},
  {1, 2},
  // Corners of the inner square.
  {3, 1},
  {3, 3},
  {1, 3},
  {1, 1},
  // Midpoints of the outer square.
  {2, 0},
  {4, 2},
  {2, 4},
  {0, 2},
  // Sidepoints of the outer square.
  {3, 0},
  {4, 1},
  {4, 3},
  {3, 4},
  {1, 4},
  {0, 3},
  {0, 1},
  {1, 0},
  // Corners of the outer square.
  {4, 0},
  {4, 4},
  {0, 4},
  {0, 0},
};

static constexpr i32 game_won_score = 100000;

MINMAX_Result minmax_search(MINMAX_Parameters const p) {
  bool const maximising_player = p.turn == p.player;
  // Terminal node.
  if(p.depth <= p.max_depth) {
    std::optional<Player> winner = p.c.check_winner();
    if(winner) {
      bool const opponent_won = winner.value() != p.player;
      i32 const score = opponent_won ? -game_won_score : game_won_score;
      return MINMAX_Result{score, -1, -1};
    }

    bool const draw = p.c.check_draw();
    if(draw) {
      return MINMAX_Result{0, -1, -1};
    }
  }

  // Leaf node of our search.
  if(p.depth >= p.max_depth) {
    i32 const h = heuristic(p.c, p.player);
    return MINMAX_Result{h, -1, -1};
  }

  Player const next_player = OPPONENT(p.turn);
  State const player_state = PLAYER_TO_STATE(p.turn);
  i32 alpha = p.alpha;
  i32 beta = p.beta;
  i32 best_x = -1;
  i32 best_y = -1;
  if(maximising_player) {
    for(auto const [x, y]: board_order) {
      if(p.c(x, y) != State::empty) {
        continue;
      }

      MINMAX_Parameters m{.c = p.c,
                          .player = p.player,
                          .turn = next_player,
                          .depth = p.depth + 1,
                          .max_depth = p.max_depth,
                          .alpha = alpha,
                          .beta = beta};
      m.c(x, y) = player_state;
      MINMAX_Result const result = minmax_search(m);
      if(result.value > alpha) {
        alpha = result.value;
        best_x = x;
        best_y = y;
      }

      if(alpha >= beta) {
        return MINMAX_Result{alpha, best_x, best_y};
      }
    }
    return MINMAX_Result{alpha, best_x, best_y};
  } else {
    for(auto const [x, y]: board_order) {
      if(p.c(x, y) != State::empty) {
        continue;
      }

      MINMAX_Parameters m{.c = p.c,
                          .player = p.player,
                          .turn = next_player,
                          .depth = p.depth + 1,
                          .max_depth = p.max_depth,
                          .alpha = alpha,
                          .beta = beta};
      m.c(x, y) = player_state;
      MINMAX_Result const result = minmax_search(m);
      if(result.value < beta) {
        beta = result.value;
        best_x = x;
        best_y = y;
      }

      if(alpha >= beta) {
        return MINMAX_Result{beta, best_x, best_y};
      }
    }
    return MINMAX_Result{beta, best_x, best_y};
  }
}
