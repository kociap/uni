#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/intrinsics.hpp>
#include <anton/math/math.hpp>
#include <anton/stream.hpp>

#include <tga.hpp>

using namespace std;

[[nodiscard]] static f32 calculate_entropy(
    anton::Slice<i64 const> const frequencies, i64 const total)
{
  f32 entropy = 0.0f;
  for (i64 const frequency : frequencies) {
    if (frequency <= 0) {
      continue;
    }
    f32 const p = static_cast<f32>(frequency) / static_cast<f32>(total);
    entropy -= p * anton::math::log2(p);
  }
  return entropy;
}

// get_channel
// r - 0
// g - 1
// b - 2
//
[[nodiscard]] static u8 get_channel(
    Image const& image, i32 const x, i32 const y, i32 const channel)
{
  i32 const offset = 3 * (y * image.width + x);
  return image.data[offset + channel];
}

[[nodiscard]] static u32 get_pixel_as_u32(
    Image const& image, i32 const x, i32 const y)
{
  i32 const offset = 3 * (y * image.width + x);
  u8 const r = image.data[offset];
  u8 const g = image.data[offset + 1];
  u8 const b = image.data[offset + 2];
  return (static_cast<u32>(r) << 16) | (static_cast<u32>(g) << 8)
      | static_cast<u32>(b);
}

using predictor_t = u32 (*)(u32 A, u32 B, u32 C);

Array<u8> run_predictor(Image const& image, predictor_t const predictor)
{
  Array<u8> predictions(image.width * image.height);

  for (i32 y = 1; y < image.height; y += 1) {
    for (i32 x = 1; x < image.width; x += 1) {
      u32 const A = get_pixel_as_u32(image, x - 1, y);
      u32 const B = get_pixel_as_u32(image, x, y - 1);
      u32 const C = get_pixel_as_u32(image, x - 1, y - 1);
      u32 const prediction = predictor(A, B, C);
      u32 const pixel = get_pixel_as_u32(image, x, y);
      predictions[y * image.width + x] = (pixel - prediction) % 256;
    }
  }

  // Prediction at border as a separate step.

  // Top-left pixel.
  predictions[0] = (0 - predictor(0, 0, 0)) % 256;

  // Top border.
  for (i32 x = 1; x < image.width; x += 1) {
    u32 const A = get_pixel_as_u32(image, x - 1, 0);
    u32 const prediction = predictor(A, 0, 0);
    u32 const pixel = get_pixel_as_u32(image, x, 0);
    predictions[x] = (pixel - prediction) % 256;
  }

  // Left border.
  for (i32 y = 1; y < image.height; y += 1) {
    u32 const B = get_pixel_as_u32(image, 0, y - 1);
    u32 const prediction = predictor(0, B, 0);
    u32 const pixel = get_pixel_as_u32(image, 0, y);
    predictions[y * image.width] = (pixel - prediction) % 256;
  }

  return predictions;
}

Array<u8> run_predictor(
    Image const& image, i32 const channel, predictor_t const predictor)
{
  Array<u8> predictions(image.width * image.height);

  for (i32 y = 1; y < image.height; y += 1) {
    for (i32 x = 1; x < image.width; x += 1) {
      u32 const A = get_channel(image, x - 1, y, channel);
      u32 const B = get_channel(image, x, y - 1, channel);
      u32 const C = get_channel(image, x - 1, y - 1, channel);
      u32 const prediction = predictor(A, B, C);
      u32 const pixel = get_channel(image, x, y, channel);
      predictions[y * image.width + x] = (pixel - prediction) % 256;
    }
  }

  // Prediction at border as a separate step.

  // Top-left pixel.
  predictions[0] = predictor(0, 0, 0);

  // Top border.
  for (i32 x = 1; x < image.width; x += 1) {
    u32 const A = get_channel(image, x - 1, 0, channel);
    u32 const prediction = predictor(A, 0, 0);
    u32 const pixel = get_channel(image, x, 0, channel);
    predictions[x] = (pixel - prediction) % 256;
  }

  // Left border.
  for (i32 y = 1; y < image.height; y += 1) {
    u32 const B = get_channel(image, 0, y - 1, channel);
    u32 const prediction = predictor(0, B, 0);
    u32 const pixel = get_channel(image, 0, y, channel);
    predictions[y * image.width] = (pixel - prediction) % 256;
  }

  return predictions;
}

[[nodiscard]] static u32 predictor_W(u32 W, u32 N, u32 NW)
{
  ANTON_UNUSED(N);
  ANTON_UNUSED(NW);
  return W;
}

[[nodiscard]] static u32 predictor_N(u32 W, u32 N, u32 NW)
{
  ANTON_UNUSED(W);
  ANTON_UNUSED(NW);
  return N;
}

[[nodiscard]] static u32 predictor_NW(u32 W, u32 N, u32 NW)
{
  ANTON_UNUSED(W);
  ANTON_UNUSED(N);
  return NW;
}

[[nodiscard]] static u32 predictor_X(u32 W, u32 N, u32 NW)
{
  return N + W - NW;
}

[[nodiscard]] static u32 predictor_Y(u32 W, u32 N, u32 NW)
{
  return N + (W - NW) / 2;
}

[[nodiscard]] static u32 predictor_Z(u32 W, u32 N, u32 NW)
{
  return W + (N - NW) / 2;
}

[[nodiscard]] static u32 predictor_P(u32 W, u32 N, u32 NW)
{
  ANTON_UNUSED(NW);
  return (W + N) / 2;
}

[[nodiscard]] static u32 predictor_new(u32 W, u32 N, u32 NW)
{
  u32 const max = math::max(N, W);
  u32 const min = math::min(N, W);
  if (NW >= max) {
    return max;
  } else if (NW <= min) {
    return min;
  } else {
    return W + N - NW;
  }
}

i32 main(i32 argc, char** argv)
{
  STDOUT_Stream stdout;
  STDERR_Stream stderr;
  if (argc < 2) {
    anton::String_View executable(argv[0]);
    stderr.write(anton::format("{} <infile>\n"_sv, executable));
    return -1;
  }

  anton::String const infilename(argv[1]);
  anton::fs::Input_File_Stream infile(infilename);
  if (!infile.is_open() || infile.error()) {
    stderr.write(
        anton::format("error: could not open infile '{}'\n"_sv, infilename));
    return -1;
  }

  infile.seek(Seek_Dir::end, 0);
  i64 const infile_size = infile.tell();
  infile.seek(Seek_Dir::beg, 0);
  Array<u8> tga_data { reserve, infile_size };
  tga_data.force_size(infile_size);
  infile.read(tga_data.data(), infile_size);

  Expected<Image, Error> import_result = import_tga(tga_data);
  if (!import_result) {
    stderr.write(anton::format("error: {}\n"_sv, import_result.error()));
    return -1;
  }

  Image& image = import_result.value();
  if (image.pixel_format != Image_Pixel_Format::rgb8) {
    return 1;
  }
  {
    Array<i64> frequencies(256 * 256 * 256, 0);
    for (i32 y = 0; y < image.height; y += 1) {
      for (i32 x = 0; x < image.width; x += 1) {
        u32 const pixel = get_pixel_as_u32(image, x, y);
        frequencies[pixel] += 1;
      }
    }
    i64 const total = image.width * image.height;
    f32 const image_entropy = calculate_entropy(frequencies, total);
    stdout.write(format("image entropy: {}\n", image_entropy));
  }

  for (i32 channel = 0; channel < 3; channel += 1) {
    Array<i64> frequencies(256, 0);
    for (i32 y = 0; y < image.height; y += 1) {
      for (i32 x = 0; x < image.width; x += 1) {
        u8 const value = get_channel(image, x, y, channel);
        frequencies[value] += 1;
      }
    }
    i64 const total = image.width * image.height;
    f32 const entropy = calculate_entropy(frequencies, total);
    stdout.write(format("channel {} entropy: {}\n", channel, entropy));
  }

  struct Named_Predictor {
    String_View name;
    predictor_t fn;
  };

  Named_Predictor predictors[] = {
    { "predictor_W", predictor_W },
    { "predictor_N", predictor_N },
    { "predictor_NW", predictor_NW },
    { "predictor_X", predictor_X },
    { "predictor_Y", predictor_Y },
    { "predictor_Z", predictor_Z },
    { "predictor_P", predictor_P },
    { "predictor_new", predictor_new },
  };

  for (Named_Predictor predictor : predictors) {
    Array<u8> predictions = run_predictor(image, predictor.fn);
    Array<i64> frequencies(256, 0);
    for (u8 const v : predictions) {
      frequencies[v] += 1;
    }
    f32 const entropy = calculate_entropy(frequencies, predictions.size());
    stdout.write(format("predictor {} entropy: {}\n", predictor.name, entropy));
  }

  for (i32 channel = 0; channel < 3; ++channel) {
    f32 best_entropy = math::largest_representable;
    String_View best_predictor;
    for (Named_Predictor predictor : predictors) {
      Array<u8> predictions = run_predictor(image, channel, predictor.fn);
      Array<i64> frequencies(256, 0);
      for (u8 const v : predictions) {
        frequencies[v] += 1;
      }
      f32 const entropy = calculate_entropy(frequencies, predictions.size());
      if (entropy < best_entropy) {
        best_entropy = entropy;
        best_predictor = predictor.name;
      }
    }
    stdout.write(format("channel {} best predictor {} with entropy: {}\n",
        channel, best_predictor, best_entropy));
  }

  return 0;
}
