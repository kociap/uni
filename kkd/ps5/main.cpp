#include "anton/string_view.hpp"
#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/memory/core.hpp>
#include <anton/types.hpp>

#include <tga.hpp>
#include <types.hpp>

static constexpr i64 pixel_channels = 3;
static constexpr i64 vector_elements = pixel_channels;

struct Vector {
  i64 elements[vector_elements] = {};

  Vector() = default;

  Vector& operator+=(Vector const& rhs)
  {
    for(i64 i = 0; i < vector_elements; i += 1) {
      elements[i] += rhs.elements[i];
    }
    return *this;
  }

  Vector& operator/=(i64 const value)
  {
    for(i64 i = 0; i < vector_elements; i += 1) {
      elements[i] /= value;
    }
    return *this;
  }
};

[[nodiscard]] Vector operator+(Vector lhs, Vector const& rhs)
{
  lhs += rhs;
  return lhs;
}

[[nodiscard]] Vector operator-(Vector lhs, Vector const& rhs)
{
  for(i64 i = 0; i < vector_elements; i += 1) {
    lhs.elements[i] -= rhs.elements[i];
  }
  return lhs;
}

[[nodiscard]] i64 length_squared(Vector const& v)
{
  i64 result = 0;
  for(i64 v: v.elements) {
    result += v * v;
  }
  return result;
}

[[nodiscard]] i64 distance(Vector const& x, Vector const& y)
{
  return length_squared(x - y);
}

[[nodiscard]] Vector vector_mean(Slice<Vector const> const vectors)
{
  Vector y;
  for(Vector const& x: vectors) {
    y += x;
  }
  y /= vectors.size();
  return y;
}

void converge_LBG(Slice<Vector> const representation,
                  Slice<Vector const> const data, f64 const eps)
{
  i64 const M = representation.size();
  Array<Array<Vector>> regions{M, {}};
  // Distortion is usually divided by N, however, since we only care about the
  // ratio (D^{k} - D^{k-1}) / D^k, we may eliminate the division by N.
  i64 D = 0;
  while(true) {
    // Find the new quantisation regions.
    for(i64 i = 0; i < M; i += 1) {
      Array<Vector>& region = regions[i];
      region.clear();
      Vector const& y_i = representation[i];
      for(Vector const& x: data) {
        i64 const d_i = distance(x, y_i);
        bool in_region = true;
        for(i64 j = 0; j < M; j += 1) {
          if(j == i) {
            continue;
          }

          Vector const& y_j = representation[j];
          i64 const d_j = distance(x, y_j);
          if(d_i >= d_j) {
            in_region = false;
            break;
          }
        }

        if(in_region) {
          region.push_back(x);
        }
      }
    }

    // Calculate the next distortion.
    i64 next_D = 0;
    for(i64 i = 0; i < M; i += 1) {
      Vector const& y = representation[i];
      for(Vector const& x: regions[i]) {
        next_D += length_squared(x - y);
      }
    }

    f64 const delta_D = static_cast<f64>(next_D - D) / static_cast<f64>(next_D);
    D = next_D;
    if(delta_D < eps) {
      return;
    }

    // Calculate the new representation vectors which are the means of their
    // respective regions.
    for(i64 i = 0; i < M; i += 1) {
      Vector& y = representation[i];
      Array<Vector>& region = regions[i];
      if(region.size() > 0) {
        y = vector_mean(region);
      }
    }
  }
}

[[nodiscard]] Array<Vector>
generate_representation(Slice<Vector const> const data, i64 const size,
                        f64 const eps)
{
  // Initialise perturbation vector to all 1s.
  Vector perturbation;
  for(i64& e: perturbation.elements) {
    e = 1;
  }

  Array<Vector> representation{reserve, size};
  Vector const initial = vector_mean(data);
  representation.push_back(initial);
  converge_LBG(representation, data, eps);
  while(representation.size() < size) {
    for(Vector const& v: representation) {
      if(representation.size() >= size) {
        break;
      }

      representation.push_back(v + perturbation);
    }

    converge_LBG(representation, data, eps);
  }
  return representation;
}

[[nodiscard]] Vector
find_closest_representation(Slice<Vector const> const representation,
                            Vector const v)
{
  i64 best_distance = limits::maximum_i64;
  Vector const* best_vector = nullptr;
  for(Vector const& c: representation) {
    i64 const d = distance(c, v);
    if(d < best_distance) {
      best_vector = &c;
      best_distance = d;
    }
  }
  return *best_vector;
}

[[nodiscard]] Array<Vector> quantise(Slice<Vector const> const representation,
                                     i64 const width, i64 const height,
                                     Slice<Vector const> const data,
                                     i64 const block_size)
{
  if(representation.size() < 1) {
    return {};
  }

  Array<Vector> quantised{reserve, data.size()};
  quantised.force_size(data.size());
  for(i64 my = 0; my < height; my += block_size) {
    for(i64 mx = 0; mx < width; mx += block_size) {
      // Find the average color of the block.
      i64 count = 0;
      Vector block;
      for(i64 dy = 0; dy < block_size; dy += 1) {
        for(i64 dx = 0; dx < block_size; dx += 1) {
          i64 const x = mx + dx;
          i64 const y = my + dy;
          if(x >= width || y >= height) {
            // Do not include when outside the image.
            continue;
          }
          i64 const offset = y * width + x;
          block += data[offset];
          count += 1;
        }
      }
      block /= count;
      Vector const color = find_closest_representation(representation, block);
      // Assign the average color to every pixel in this block.
      for(i64 dy = 0; dy < block_size; dy += 1) {
        for(i64 dx = 0; dx < block_size; dx += 1) {
          i64 const x = mx + dx;
          i64 const y = my + dy;
          if(x >= width || y >= height) {
            continue;
          }

          i64 const offset = y * width + x;
          quantised[offset] = color;
        }
      }
    }
  }
  return quantised;
}

[[nodiscard]] Array<Vector> vectorise(i64 const width, i64 const height,
                                      Slice<u8 const> const data)
{
  Array<Vector> vectorised{reserve, data.size() / pixel_channels};
  for(i64 y = 0; y < height; y += 1) {
    for(i64 x = 0; x < width; x += 1) {
      i64 const offset = pixel_channels * (y * width + x);
      Vector pixel;
      for(i64 c = 0; c < pixel_channels; c += 1) {
        pixel.elements[c] = data[offset + c];
      }
      vectorised.push_back(pixel);
    }
  }
  return vectorised;
}

[[nodiscard]] Array<u8> devectorise(i64 const width, i64 const height,
                                    Slice<Vector const> const data)
{
  Array<u8> devectorised{reserve, data.size() * pixel_channels};
  for(i64 y = 0; y < height; y += 1) {
    for(i64 x = 0; x < width; x += 1) {
      i64 const offset = y * width + x;
      Vector const pixel = data[offset];
      for(i64 c = 0; c < pixel_channels; c += 1) {
        devectorised.push_back(pixel.elements[c]);
      }
    }
  }
  return devectorised;
}

[[nodiscard]] f64 calculate_MSE(Slice<Vector const> const predicted,
                                Slice<Vector const> const source)
{
  i64 value = 0;
  for(i64 i = 0; i < predicted.size(); i += 1) {
    value += length_squared(predicted[i] - source[i]);
  }
  return static_cast<f64>(value) / static_cast<f64>(predicted.size());
}

i32 main(i32 const argc, char** const argv)
{
  STDOUT_Stream stdout;
  STDERR_Stream stderr;
  if(argc < 5) {
    anton::String_View executable(argv[0]);
    stderr.write(anton::format(
      "{} <infile> <outfile> <colors_exponent> <block_size>\n"_sv, executable));
    return -1;
  }

  i64 const block_size = math::max(str_to_i64(argv[4]), (i64)1);
  i64 const colors_exponent = math::max(str_to_i64(argv[3]), (i64)1);
  if(colors_exponent > 24) {
    stderr.write(anton::format("error: colors exponent must be at most 24\n"_sv,
                               colors_exponent));
    return -1;
  }
  i64 const colors = 1 << colors_exponent;

  anton::String const infilename(argv[1]);
  anton::fs::Input_File_Stream infile(infilename);
  if(!infile.is_open() || infile.error()) {
    stderr.write(
      anton::format("error: could not open infile '{}'\n"_sv, infilename));
    return -1;
  }

  infile.seek(Seek_Dir::end, 0);
  i64 const infile_size = infile.tell();
  infile.seek(Seek_Dir::beg, 0);
  Array<u8> tga_data{reserve, infile_size};
  tga_data.force_size(infile_size);
  infile.read(tga_data.data(), infile_size);
  infile.close();

  Expected<Image, Error> import_result = import_tga(tga_data);
  if(!import_result) {
    stderr.write(anton::format("error: {}\n"_sv, import_result.error()));
    return -1;
  }

  Image& image = import_result.value();
  if(image.pixel_format != Image_Pixel_Format::rgb8) {
    return 1;
  }

  Array<Vector> data = vectorise(image.width, image.height, image.data);
  Array<Vector> representation = generate_representation(data, colors, 0.1);
  Array<Vector> quantised_data =
    quantise(representation, image.width, image.height, data, block_size);
  Array<u8> quantised_devectorised_data =
    devectorise(image.width, image.height, quantised_data);

  anton::String const outfilename(argv[2]);
  anton::fs::Output_File_Stream outfile(outfilename);
  if(!outfile.is_open()) {
    stderr.write(
      anton::format("error: could not open outfile '{}'\n"_sv, outfilename));
    return -1;
  }

  // TGA kinda dumb in that it stores pixels in little-endian (BGR instead of
  // RGB), hence we have to swap the bytes around.
  for(i64 i = 0; i < quantised_devectorised_data.size(); i += pixel_channels) {
    swap(quantised_devectorised_data[i], quantised_devectorised_data[i + 2]);
  }
  outfile.write(tga_data.data(), 18);
  outfile.write(quantised_devectorised_data.data(),
                quantised_devectorised_data.size());
  outfile.close();

  f64 const MSE = calculate_MSE(quantised_data, data);
  stdout.write(anton::format("MSE: {}\n"_sv, MSE));

  return 0;
}
