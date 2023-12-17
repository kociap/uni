#pragma once

#include <anton/array.hpp>
#include <anton/slice.hpp>
#include <anton/string.hpp>
#include <anton/types.hpp>

using namespace anton;

using Error = anton::String;

using pixel_element_t = i32;

struct Pixel {
  pixel_element_t r = 0;
  pixel_element_t g = 0;
  pixel_element_t b = 0;

  [[nodiscard]] Pixel normalised() const
  {
    return {math::min(255, math::max(0, r)), math::min(255, math::max(0, g)),
            math::min(255, math::max(0, b))};
  }
};

[[nodiscard]] inline Pixel operator+(Pixel lhs, Pixel rhs)
{
  return {static_cast<pixel_element_t>(lhs.r + rhs.r),
          static_cast<pixel_element_t>(lhs.g + rhs.g),
          static_cast<pixel_element_t>(lhs.b + rhs.b)};
}

[[nodiscard]] inline Pixel operator-(Pixel lhs, Pixel rhs)
{
  return {static_cast<pixel_element_t>(lhs.r - rhs.r),
          static_cast<pixel_element_t>(lhs.g - rhs.g),
          static_cast<pixel_element_t>(lhs.b - rhs.b)};
}

[[nodiscard]] inline Pixel operator/(Pixel lhs, i32 const value)
{
  return {static_cast<pixel_element_t>(lhs.r / value),
          static_cast<pixel_element_t>(lhs.g / value),
          static_cast<pixel_element_t>(lhs.b / value)};
}

struct Custom_Header {
  i32 width;
  i32 height;
  i32 low_size;
  i32 high_size;
  u8 tga_header[18];
};
