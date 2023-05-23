#pragma once

using u8 = unsigned char;

using i32 = int;

constexpr i32 maximum_i32 = 0x7FFFFFFF;
constexpr i32 minimum_i32 = 0x80000000;

enum struct Player { x = 0, o = 1 };

#define OPPONENT(p) static_cast<Player>(!static_cast<bool>(p))

struct Point {
  i32 x;
  i32 y;
};
