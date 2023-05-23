#include <heuristic.hpp>

#include <span>
#include <vector>

static constexpr i32 score_open_cross = 200;
static constexpr i32 score_closed_cross = 500;
static constexpr i32 score_skew_cross = 1000;
static constexpr i32 score_skew_right_cross = 1000;
static constexpr i32 score_skew_left_cross = 1000;
static constexpr i32 score_open_4 = 50;
static constexpr i32 score_open_5 = -800;
static constexpr i32 score_opponent_open_4 = 10000;
static constexpr i32 score_opponent_open_5 = -300;
static constexpr i32 score_opponent_closed_cross = 10000;
static constexpr i32 score_opponent_open_cross = 1000;
static constexpr i32 score_opponent_skew_cross = 10000;
static constexpr i32 score_opponent_skew_right_cross = 10000;
static constexpr i32 score_opponent_skew_left_cross = 10000;

// TODO: Open cross with an option to set up.

i32 heuristic(Configuration const& c, Player const player) {
  // Nomenclature:
  // - open 4
  //   3 pieces in a line separated by an empty square allowing to play
  //   4-in-a-line.
  //   Example: X|X| |X
  // - open cross
  //   2 pieces intersecting at a square with opponent's open 3.
  //   Example:
  //
  // - open 5
  //   Unfavourable position for us forcing us to play 3-in-a-line if things get
  //   messy. Open 5 will be counted twice, hence we have to adjust its score
  //   accordingly.
  //   Example:
  //   X| |X| |X
  //
  // We choose to be pessimistic in our approach and assume that the opponent is
  // smart and will play to the best of their abilities. Therefore, we try to
  // avoid single-move threats and try to prevent zugzwang on our side.
  // - Positions which force us to place 3-in-a-line are highly unfavourable for
  //   us and must be prevented at all cost. Those are the highest priority for
  //   us.
  // - Positions in which the opponent has an open 3 are equally disadvantageous
  //   as they will result in our loss in the very next turn. Along with the
  //   above, those are the highest priority for us.
  // - Positions in which we have multiple threats are highly favourable for us
  //   allowing us to attack from multiple sides and eventually win.
  // - We do not want to block opponent's bad moves, e.g. placing 3 in a line.
  // - Positions in which we have an open 3 are somewhat favourable, but we
  //   assume the opponent will block our move the very next turn, hence we
  //   score them 0.
  // - If there are no immediate threats or better moves for us, we want to
  //   block any attempts at setting up open 3s while ensuring we do not make
  //   tragic moves ourselves.
  // - We want to build up multiple 2-in-a-line as that expands our capabilities
  //   to later play open 3s and set up threats.

  Player const opponent = OPPONENT(player);
  State const player_state = PLAYER_TO_STATE(player);
  State const opponent_state = PLAYER_TO_STATE(opponent);
  std::vector<Point> available_moves;
  for(i32 y = 0; y < Configuration::height; y += 1) {
    for(i32 x = 0; x < Configuration::width; x += 1) {
      if(c(x, y) == State::empty) {
        available_moves.push_back(Point{x, y});
      }
    }
  }

  struct Pattern_Element {
    State state;
    i32 dx;
    i32 dy;
  };

  auto check_pattern =
    [](Configuration const& c, i32 const x, i32 const y,
       std::span<Pattern_Element const> const pattern) -> bool {
#define CHECK_ROTATION(x_expr, y_expr)            \
  {                                               \
    bool found = true;                            \
    for(auto const [state, dx, dy]: pattern) {    \
      i32 const px = x + x_expr;                  \
      i32 const py = y + y_expr;                  \
      if(px >= Configuration::width || px < 0 ||  \
         py >= Configuration::height || py < 0) { \
        found = false;                            \
        break;                                    \
      }                                           \
                                                  \
      State const s = c(px, py);                  \
      if(s != state) {                            \
        found = false;                            \
        break;                                    \
      }                                           \
    }                                             \
    if(found) {                                   \
      return true;                                \
    }                                             \
  }
    // 0deg
    CHECK_ROTATION(dx, dy);
    // 45deg
    CHECK_ROTATION(dx - dy, dx + dy);
    // 90deg
    CHECK_ROTATION(dy, dx);
    // 135deg
    CHECK_ROTATION(-dx - dy, dx - dy);
    // 180deg
    CHECK_ROTATION(-dx, -dy);
    // 225deg
    CHECK_ROTATION(-dx + dy, -dx - dy);
    // 270deg
    CHECK_ROTATION(-dy, -dx);
    // 315deg
    CHECK_ROTATION(dx + dy, -dx + dy);

    // Horizontal reflection.
    // 0deg
    CHECK_ROTATION(-dx, dy);
    // 45deg
    CHECK_ROTATION(-dx - dy, -dx + dy);
    // 90deg
    CHECK_ROTATION(dy, -dx);
    // 135deg
    CHECK_ROTATION(dx - dy, -dx - dy);
    // 180deg
    CHECK_ROTATION(dx, -dy);
    // 225deg
    CHECK_ROTATION(dx + dy, dx - dy);
    // 270deg
    CHECK_ROTATION(-dy, dx);
    // 315deg
    CHECK_ROTATION(-dx + dy, dx + dy);

    return false;
#undef CHECK_ROTATION
  };

  Pattern_Element pattern_open_cross[] = {{player_state, 0, 1},
                                          {player_state, 0, 2},
                                          {opponent_state, 1, 0},
                                          {opponent_state, -1, 0}};
  Pattern_Element pattern_opponent_open_cross[] = {{opponent_state, 0, 1},
                                                   {opponent_state, 0, 2},
                                                   {player_state, 1, 0},
                                                   {player_state, -1, 0}};
  Pattern_Element pattern_closed_cross[] = {{player_state, 0, 1},
                                            {player_state, 0, 2},
                                            {player_state, 0, -1},
                                            {opponent_state, 1, 0},
                                            {opponent_state, -1, 0}};
  Pattern_Element pattern_opponent_closed_cross[] = {{opponent_state, 0, 1},
                                                     {opponent_state, 0, 2},
                                                     {opponent_state, 0, -1},
                                                     {player_state, 1, 0},
                                                     {player_state, -1, 0}};
  Pattern_Element pattern_skew_cross[] = {{player_state, -1, -1},
                                          {player_state, 1, 1},
                                          {player_state, 2, 2},
                                          {opponent_state, -1, 0},
                                          {opponent_state, 1, 0}};
  Pattern_Element pattern_opponent_skew_cross[] = {{opponent_state, -1, -1},
                                                   {opponent_state, 1, 1},
                                                   {opponent_state, 2, 2},
                                                   {player_state, -1, 0},
                                                   {player_state, 1, 0}};
  Pattern_Element pattern_skew_left_cross[] = {{player_state, -1, -1},
                                               {player_state, 1, 1},
                                               {player_state, 2, 2},
                                               {opponent_state, -1, 0},
                                               {opponent_state, -1, 0}};
  Pattern_Element pattern_opponent_skew_left_cross[] = {
    {opponent_state, -1, -1},
    {opponent_state, 1, 1},
    {opponent_state, 2, 2},
    {player_state, -1, 0},
    {player_state, -1, 0}};
  Pattern_Element pattern_skew_right_cross[] = {{player_state, -1, -1},
                                                {player_state, 1, 1},
                                                {player_state, 2, 2},
                                                {opponent_state, 1, 0},
                                                {opponent_state, 2, 0}};
  Pattern_Element pattern_opponent_skew_right_cross[] = {
    {opponent_state, -1, -1},
    {opponent_state, 1, 1},
    {opponent_state, 2, 2},
    {player_state, 1, 0},
    {player_state, 2, 0}};
  Pattern_Element pattern_open_4[] = {
    {player_state, 2, 0},
    {player_state, 1, 0},
    {player_state, -1, 0},
  };
  Pattern_Element pattern_opponent_open_4[] = {
    {opponent_state, 2, 0},
    {opponent_state, 1, 0},
    {opponent_state, -1, 0},
  };
  Pattern_Element pattern_open_5[] = {
    {player_state, -1, 0},
    {player_state, 1, 0},
    {State::empty, 2, 0},
    {player_state, 3, 0},
  };
  Pattern_Element pattern_opponent_open_5[] = {
    {opponent_state, -1, 0},
    {opponent_state, 1, 0},
    {State::empty, 2, 0},
    {opponent_state, 3, 0},
  };

  i32 score = 0;
  for(auto const [x, y]: available_moves) {
    bool const opponent_closed_cross =
      check_pattern(c, x, y, pattern_opponent_closed_cross);
    if(opponent_closed_cross) {
      score -= score_opponent_closed_cross;
      continue;
    }

    bool const opponent_open_4 =
      check_pattern(c, x, y, pattern_opponent_open_4);
    if(opponent_open_4) {
      score -= score_opponent_open_4;
      continue;
    }

    bool const opponent_skew_cross =
      check_pattern(c, x, y, pattern_opponent_skew_cross);
    if(opponent_skew_cross) {
      score -= score_opponent_skew_cross;
      continue;
    }

    bool const opponent_skew_right_cross =
      check_pattern(c, x, y, pattern_opponent_skew_right_cross);
    if(opponent_skew_right_cross) {
      score -= score_opponent_skew_right_cross;
      continue;
    }

    bool const opponent_skew_left_cross =
      check_pattern(c, x, y, pattern_opponent_skew_left_cross);
    if(opponent_skew_left_cross) {
      score -= score_opponent_skew_left_cross;
      continue;
    }

    bool const skew_cross = check_pattern(c, x, y, pattern_skew_cross);
    if(skew_cross) {
      score -= score_skew_cross;
      continue;
    }

    bool const skew_right_cross =
      check_pattern(c, x, y, pattern_skew_right_cross);
    if(skew_right_cross) {
      score -= score_skew_right_cross;
      continue;
    }

    bool const skew_left_cross =
      check_pattern(c, x, y, pattern_skew_left_cross);
    if(skew_left_cross) {
      score -= score_skew_left_cross;
      continue;
    }

    bool const closed_cross = check_pattern(c, x, y, pattern_closed_cross);
    if(closed_cross) {
      score += score_closed_cross;
    }

    if(!closed_cross) {
      bool const open_cross = check_pattern(c, x, y, pattern_open_cross);
      if(open_cross) {
        score += score_open_cross;
      }
    }

    bool const opponent_open_cross =
      check_pattern(c, x, y, pattern_opponent_open_cross);
    if(opponent_open_cross) {
      score -= score_opponent_open_cross;
    }

    bool const open_4 = check_pattern(c, x, y, pattern_open_4);
    if(open_4) {
      score += score_open_4;
    }

    bool const open_5 = check_pattern(c, x, y, pattern_open_5);
    if(open_5) {
      score -= score_open_5;
    }

    bool const opponent_open_5 =
      check_pattern(c, x, y, pattern_opponent_open_5);
    if(opponent_open_5) {
      score -= score_opponent_open_5;
    }
  }

  // std::vector<Point> player_reserve = calculate_reserve(player);
  // std::vector<Point> opponent_reserve = calculate_reserve(OPPONENT(player));
  // i32 common_reserve = 0;

  // i32 const score = player_weight * player_reserve.size() -
  //                   opponent_weight * opponent_reserve.size() -
  //                   common_weight * common_reserve;
  return score;
}