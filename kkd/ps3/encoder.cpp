#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/optional.hpp>
#include <anton/slice.hpp>

#include <LZW.hpp>
#include <universal.hpp>

using namespace anton::literals;

[[nodiscard]] f32 calculate_entropy(anton::Slice<i64 const> const frequencies,
                                    i64 const total)
{
  f32 entropy = 0.0f;
  for(i64 const frequency: frequencies) {
    if(frequency <= 0) {
      continue;
    }
    f32 const p = static_cast<f32>(frequency) / static_cast<f32>(total);
    entropy -= p * anton::math::log2(p);
  }
  return entropy;
}

int main(int argc, char** argv)
{
  anton::STDOUT_Stream stdout_stream;
  anton::STDERR_Stream stderr_stream;
  if(argc < 3) {
    anton::String_View executable(argv[0]);
    stderr_stream.write(
      anton::format("{} <infile> <outfile>\n"_sv, executable));
    return -1;
  }

  anton::String const infilename(argv[1]);
  anton::fs::Input_File_Stream infile(infilename);
  if(infile.error()) {
    stderr_stream.write(
      anton::format("error: could not open infile '{}'\n"_sv, infilename));
    return -1;
  }

  anton::String const outfilename(argv[2]);
  anton::fs::Output_File_Stream outfile(outfilename);
  if(!outfile.is_open()) {
    stderr_stream.write(
      anton::format("error: could not open outfile '{}'\n"_sv, outfilename));
    return -1;
  }

  Encoding_Statistics const statistics =
    compress_LZW(infile, outfile, encode_fibonacci);
  f32 const entropy =
    calculate_entropy(statistics.frequencies, statistics.read);
  f32 const written = static_cast<f32>(statistics.written);
  f32 const read = static_cast<f32>(statistics.read);
  f32 const ratio = read / written;
  f32 const saving = 1.0f - written / read;
  f32 const avg_code_length = 8.0f * written / read;
  stdout_stream.write(
    anton::format("entropy {}; ratio {}; saving {}; avg code length: {}\n"_sv,
                  entropy, ratio, saving, avg_code_length));
  return 0;
}
