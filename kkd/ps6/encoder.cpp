#include <anton/array.hpp>
#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/slice.hpp>

#include <tga.hpp>
#include <types.hpp>

struct Filters {
  Array<Pixel> low;
  Array<Pixel> high;
};

[[nodiscard]] static Filters filter(Slice<Pixel const> const data)
{
  Filters filters;
  filters.low.ensure_capacity(data.size());
  filters.high.ensure_capacity(data.size());
  Pixel previous;
  for(Pixel const p: data) {
    Pixel const lf = p / 2 - previous / 2; // 1/2 p - 1/2 prev
    Pixel const hf = p - lf; // p - 1/2p + 1/2 prev = 1/2 p + 1/2 prev
    previous = p;
    filters.low.push_back(lf);
    filters.high.push_back(hf);
  }
  return filters;
}

struct Channel_Iterator {
public:
  Channel_Iterator() = default;
  Channel_Iterator(Pixel* pixel, u8 channel): pixel(pixel), channel(channel) {}

  Channel_Iterator& operator++()
  {
    ++pixel;
    return *this;
  }

  [[nodiscard]] pixel_element_t& operator*()
  {
    return reinterpret_cast<pixel_element_t*>(pixel)[channel];
  }

  [[nodiscard]] bool operator==(Channel_Iterator const& other) const
  {
    return pixel == other.pixel;
  }

  [[nodiscard]] bool operator!=(Channel_Iterator const& other) const
  {
    return pixel != other.pixel;
  }

private:
  Pixel* pixel = nullptr;
  u8 channel = 0;
};

using Vector = i32;

[[nodiscard]] static i64 length_squared(Vector v)
{
  return v * v;
}

[[nodiscard]] static i64 distance(Vector a, Vector b)
{
  return math::abs(a - b);
}

template<typename Iterator>
[[nodiscard]] static Vector vector_mean(Iterator const begin,
                                        Iterator const end)
{
  Vector y = {};
  i64 count = 0;
  for(Vector const x: Range(begin, end)) {
    y += x;
    count += 1;
  }
  y /= count;
  return y;
}

[[nodiscard]] static Vector apply_perturbation(Vector const v)
{
  return v < 128 ? v + 2 : v - 2;
}

template<typename Iterator>
static void converge_LBG(Slice<Vector> const representation,
                         Iterator const begin, Iterator const end,
                         f64 const eps)
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
      for(Vector const x: Range(begin, end)) {
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
        y = vector_mean(region.begin(), region.end());
      }
    }
  }
}

// generate_representation
//
// Parameters:
// size - the number of vectors in the representation (colors).
//
template<typename Iterator>
[[nodiscard]] static Array<Vector>
generate_representation(Iterator const begin, Iterator const end,
                        i64 const size, f64 const eps)
{
  Array<Vector> representation{reserve, size};
  Vector const initial = vector_mean(begin, end);
  representation.push_back(initial);
  converge_LBG(representation, begin, end, eps);
  while(representation.size() < size) {
    for(Vector const& v: representation) {
      if(representation.size() >= size) {
        break;
      }

      Vector const perturbed = apply_perturbation(v);
      representation.push_back(perturbed);
    }

    converge_LBG(representation, begin, end, eps);
  }
  return representation;
}

[[nodiscard]] static Vector
find_closest_representation(Slice<Vector const> const representation,
                            Vector const v)
{
  ANTON_ASSERT(representation.size() > 0,
               "representation must contain at least one element");
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

template<typename Iterator>
static void quantise(Slice<Vector const> const representation, Iterator begin,
                     Iterator const end)
{
  ANTON_ASSERT(representation.size() > 0,
               "representation must contain at least one element");
  for(auto& v: Range(begin, end)) {
    Vector const color = find_closest_representation(representation, v);
    v = color;
  }
}

[[nodiscard]] static Array<Pixel> difference(Slice<Pixel const> const data)
{
  Array<Pixel> pixels{reserve, data.size()};
  pixels.force_size(data.size());
  pixels[0] = data[0];
  Pixel const* prev = data.begin();
  Pixel const* current = data.begin() + 1;
  Pixel const* const end = data.end();
  Pixel* dst = pixels.data() + 1;
  while(current != end) {
    *dst = *current - *prev;
    ++current;
    ++prev;
    ++dst;
  }
  return pixels;
}

// [[nodiscard]] f64 calculate_MSE(Slice<Pixel const> const predicted,
//                                 Slice<Pixel const> const source)
// {
//   i64 value = 0;
//   for(i64 i = 0; i < predicted.size(); i += 1) {
//     value += length_squared(predicted[i] - source[i]);
//   }
//   return static_cast<f64>(value) / static_cast<f64>(predicted.size());
// }

i32 main(i32 const argc, char** const argv)
{
  STDOUT_Stream stdout;
  STDERR_Stream stderr;
  if(argc < 4) {
    anton::String_View executable(argv[0]);
    stderr.write(
      anton::format("{} <infile> <outfile> <bits>\n"_sv, executable));
    return 1;
  }

  i64 const bits = math::max(str_to_i64(argv[3]), (i64)1);
  if(bits > 7) {
    stderr.write(
      anton::format("error: bits must be no more than 7\n"_sv, bits));
    return 1;
  }

  anton::String const infilename(argv[1]);
  anton::fs::Input_File_Stream infile(infilename);
  if(!infile.is_open() || infile.error()) {
    stderr.write(
      anton::format("error: could not open infile '{}'\n"_sv, infilename));
    return 1;
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
    return 1;
  }

  Image& image = import_result.value();
  if(image.pixel_format != Image_Pixel_Format::rgb8) {
    return 1;
  }

  Array<Pixel> pixels{reserve, image.data.size() / 3};
  pixels.force_size(image.data.size() / 3);
  {
    // We could as well have reintepreted the data, but for testing purposes
    // pixel_element_t was not the same as u8, hence the copy.
    pixel_element_t* dst = reinterpret_cast<pixel_element_t*>(pixels.begin());
    copy(image.data.begin(), image.data.end(), dst);
  }
  Filters filtered = filter(pixels);
  Array<Pixel> differenced_low = difference(filtered.low);
  Array<Pixel>& quantised_high = filtered.high;
  // Quantise channels.
  {
    Channel_Iterator const end(quantised_high.end(), 0);
    Channel_Iterator const begin_r(quantised_high.begin(), 0);
    Channel_Iterator const begin_g(quantised_high.begin(), 1);
    Channel_Iterator const begin_b(quantised_high.begin(), 2);
    Array<Vector> representation_r =
      generate_representation(begin_r, end, 1 << bits, 0.1);
    quantise(representation_r, begin_r, end);
    Array<Vector> representation_g =
      generate_representation(begin_g, end, 1 << bits, 0.1);
    quantise(representation_g, begin_g, end);
    Array<Vector> representation_b =
      generate_representation(begin_b, end, 1 << bits, 0.1);
    quantise(representation_b, begin_b, end);
  }

  anton::String const outfilename(argv[2]);
  anton::fs::Output_File_Stream outfile(outfilename);
  if(!outfile.is_open()) {
    stderr.write(
      anton::format("error: could not open outfile '{}'\n"_sv, outfilename));
    return 1;
  }

  Custom_Header header{
    .width = (i32)image.width,
    .height = (i32)image.height,
    .low_size = (i32)differenced_low.size(),
    .high_size = (i32)quantised_high.size(),
  };
  memcpy(header.tga_header, tga_data.data(), 18);
  outfile.write(&header, sizeof(Custom_Header));
  outfile.write(differenced_low.data(), differenced_low.size_bytes());
  outfile.write(quantised_high.data(), quantised_high.size_bytes());
  outfile.close();

  return 0;
}
