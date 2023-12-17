#include "anton/string.hpp"
#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/iterators/range.hpp>
#include <anton/iterators/zip.hpp>

#include <types.hpp>

#include <random>

i32 main(i32 const argc, char** const argv)
{
  STDOUT_Stream stdout;
  STDERR_Stream stderr;
  if(argc < 4) {
    anton::String_View executable(argv[0]);
    stderr.write(
      anton::format("{} <probability> <infile> <outfile>\n"_sv, executable));
    return 1;
  }

  anton::String probability_str(argv[1]);
  f64 const probability = str_to_f64(probability_str);

  anton::String const infilename(argv[2]);
  anton::fs::Input_File_Stream infile(infilename);
  if(!infile.is_open() || infile.error()) {
    stderr.write(
      anton::format("error: could not open infile '{}'\n"_sv, infilename));
    return 1;
  }

  infile.seek(Seek_Dir::end, 0);
  i64 const infile_size = infile.tell();
  infile.seek(Seek_Dir::beg, 0);
  Array<u8> data{reserve, infile_size};
  data.force_size(infile_size);
  infile.read(data.data(), infile_size);
  infile.close();

  std::random_device rd;
  std::mt19937_64 g(rd());
  std::uniform_real_distribution<f64> d(0, 1);
  u8* current = data.begin();
  u8 const* const end = data.end();
  for(; current != end; ++current) {
    u8 b = *current;
    for(i32 i = 0; i < 8; i += 1) {
      f64 const v = d(g);
      if(v < probability) {
        b = b ^ ((u8)1 << i);
      }
    }
    *current = b;
  }

  anton::String const outfilename(argv[3]);
  anton::fs::Output_File_Stream outfile(outfilename);
  if(!outfile.is_open()) {
    stderr.write(
      anton::format("error: could not open outfile '{}'\n"_sv, outfilename));
    return 1;
  }
  outfile.write(data.data(), data.size_bytes());
  outfile.close();

  return 0;
}
