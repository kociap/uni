#pragma once

#include <anton/array.hpp>
#include <anton/expected.hpp>

#include <types.hpp>

enum class Image_Pixel_Format {
  grey8,
  grey16,
  grey8_alpha8,
  grey16_alpha16,
  rgb8,
  rgb16,
  rgba8,
  rgba16,
};

enum Image_Color_Space {
  gamma_encoded,
  srgb,
  linear,
};

class Image {
public:
  i64 width;
  i64 height;
  float gamma;
  Image_Pixel_Format pixel_format;
  Image_Color_Space color_space;
  Array<u8> data;

  Image() {}
  Image(i64 width, i64 height, Image_Pixel_Format pixel_format,
        Image_Color_Space color_space, float gamma, Array<u8>&& data)
    : width(width), height(height), gamma(gamma), pixel_format(pixel_format),
      color_space(color_space), data(ANTON_MOV(data))
  {
  }
};

[[nodiscard]] Expected<Image, Error> import_tga(Array<u8> const& tga_data);
