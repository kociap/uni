#pragma once

#include <types.hpp>

#include <array>
#include <optional>

enum struct State : u8 {
  empty = 0,
  x = 1,
  o = 2,
};

#define PLAYER_TO_STATE(p) static_cast<State>(static_cast<u8>(p) + 1)
#define STATE_TO_PLAYER(s) static_cast<Player>(static_cast<u8>(s) - 1)

struct Configuration {
public:
  static constexpr i32 width = 5;
  static constexpr i32 height = 5;

private:
  std::array<State, width * height> storage;

public:
  using iterator = State*;
  using const_iterator = State const*;

public:
  Configuration(): storage{} {}

  [[nodiscard]] State& operator()(i32 const x, i32 const y) {
    return storage[y * width + x];
  }

  [[nodiscard]] State operator()(i32 const x, i32 const y) const {
    return storage[y * width + x];
  }

  [[nodiscard]] iterator begin() {
    return storage.begin();
  }

  [[nodiscard]] const_iterator begin() const {
    return storage.begin();
  }

  [[nodiscard]] iterator end() {
    return storage.end();
  }

  [[nodiscard]] const_iterator end() const {
    return storage.end();
  }

  [[nodiscard]] std::optional<Player> check_winner() const {
    // We rely on the turn-based property of the game that each turn only one
    // move is made and there may not simultaneously be distinct 3-in-a-line and
    // 4-in-a-line.

    // Rows.
    for(i32 y = 0; y < height; y += 1) {
      // Check 4-in-a-line first to determine whether anyone has won.
      for(i32 x = 0; x < width - 3; x += 1) {
        State const s = (*this)(x, y);
        if(check_4(x, y, 1, 0) && s != State::empty) {
          return STATE_TO_PLAYER(s);
        }
      }

      // Now check 3-in-a-line to determine whether anyone has lost.
      for(i32 x = 0; x < width - 2; x += 1) {
        State const s = (*this)(x, y);
        if(check_3(x, y, 1, 0) && s != State::empty) {
          return OPPONENT(STATE_TO_PLAYER(s));
        }
      }
    }

    // Columns.
    for(i32 x = 0; x < width; x += 1) {
      // Check 4-in-a-line first to determine whether anyone has won.
      for(i32 y = 0; y < height - 3; y += 1) {
        State const s = (*this)(x, y);
        if(check_4(x, y, 0, 1) && s != State::empty) {
          return STATE_TO_PLAYER(s);
        }
      }

      // Now check 3-in-a-line to determine whether anyone has lost.
      for(i32 y = 0; y < width - 2; y += 1) {
        State const s = (*this)(x, y);
        if(check_3(x, y, 0, 1) && s != State::empty) {
          return OPPONENT(STATE_TO_PLAYER(s));
        }
      }
    }

    struct Line {
      i32 x;
      i32 y;
      i32 dx;
      i32 dy;
    };

    Line const ltr_diagonals[] = {
      // Long diagonal.
      {0, 0, 1, 1},
      // Short diagonals.
      {0, 1, 1, 1},
      {1, 0, 1, 1},

    };

    for(Line const d: ltr_diagonals) {
      // Check 4-in-a-line first to determine whether anyone has won.
      for(i32 x = d.x, y = d.y; x < width - 3 && y < height - 3;
          x += d.dx, y += d.dy) {
        State const s = (*this)(x, y);
        if(check_4(x, y, d.dx, d.dy) && s != State::empty) {
          return STATE_TO_PLAYER(s);
        }
      }

      // Now check 3-in-a-line to determine whether anyone has lost.
      for(i32 x = d.x, y = d.y; x < width - 2 && y < height - 2;
          x += d.dx, y += d.dy) {
        State const s = (*this)(x, y);
        if(check_3(x, y, d.dx, d.dy) && s != State::empty) {
          return OPPONENT(STATE_TO_PLAYER(s));
        }
      }
    }

    Line const rtl_diagonals[] = {
      // Long diagonal.
      {4, 0, -1, 1},
      // Short diagonals.
      {4, 1, -1, 1},
      {3, 0, -1, 1},
    };

    for(Line const d: rtl_diagonals) {
      // Check 4-in-a-line first to determine whether anyone has won.
      for(i32 x = d.x, y = d.y; x > 2 && y < height - 3; x += d.dx, y += d.dy) {
        State const s = (*this)(x, y);
        if(check_4(x, y, d.dx, d.dy) && s != State::empty) {
          return STATE_TO_PLAYER(s);
        }
      }

      // Now check 3-in-a-line to determine whether anyone has lost.
      for(i32 x = d.x, y = d.y; x > 1 && y < height - 2; x += d.dx, y += d.dy) {
        State const s = (*this)(x, y);
        if(check_3(x, y, d.dx, d.dy) && s != State::empty) {
          return OPPONENT(STATE_TO_PLAYER(s));
        }
      }
    }

    // 3-only diagonals.
    {
      i32 const x = 2;
      i32 const y = 0;
      State const s = (*this)(x, y);
      if(check_3(x, y, 1, 1) && s != State::empty) {
        return OPPONENT(STATE_TO_PLAYER(s));
      }
    }
    {
      i32 const x = 0;
      i32 const y = 2;
      State const s = (*this)(x, y);
      if(check_3(x, y, 1, 1) && s != State::empty) {
        return OPPONENT(STATE_TO_PLAYER(s));
      }
    }
    {
      i32 const x = 2;
      i32 const y = 0;
      State const s = (*this)(x, y);
      if(check_3(x, y, -1, 1) && s != State::empty) {
        return OPPONENT(STATE_TO_PLAYER(s));
      }
    }
    {
      i32 const x = 4;
      i32 const y = 2;
      State const s = (*this)(x, y);
      if(check_3(x, y, -1, 1) && s != State::empty) {
        return OPPONENT(STATE_TO_PLAYER(s));
      }
    }

    return std::nullopt;
  }

  // check_draw
  // Dumb check to verify all spaces are occupied without verifying game draw
  // conditions. To be used in conjunction with Configuration::check_winner.
  //
  [[nodiscard]] bool check_draw() const {
    for(State const s: storage) {
      if(s == State::empty) {
        return false;
      }
    }
    return true;
  }

private:
  [[nodiscard]] bool check_4(i32 const x, i32 const y, i32 const dx,
                             i32 const dy) const {
    State const s1 = (*this)(x, y);
    State const s2 = (*this)(x + dx, y + dy);
    State const s3 = (*this)(x + 2 * dx, y + 2 * dy);
    State const s4 = (*this)(x + 3 * dx, y + 3 * dy);
    return s1 == s2 && s2 == s3 && s3 == s4;
  }

  [[nodiscard]] bool check_3(i32 const x, i32 const y, i32 const dx,
                             i32 const dy) const {
    State const s1 = (*this)(x, y);
    State const s2 = (*this)(x + dx, y + dy);
    State const s3 = (*this)(x + 2 * dx, y + 2 * dy);
    return s1 == s2 && s2 == s3;
  }
};
