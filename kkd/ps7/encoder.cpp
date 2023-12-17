#include <anton/array.hpp>
#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/slice.hpp>

#include <types.hpp>

struct Hamminged {
  u8 first;
  u8 second;
};

[[nodiscard]] u8 hamming_byte(u8 const d4, u8 const d3, u8 const d2,
                              u8 const d1)
{
  u8 const vector[] = {d4, d3, d2, d1};
  // The generating matrix.
  u8 G[7][4] = {
    {1, 0, 0, 0}, //
    {1, 1, 0, 0}, //
    {0, 1, 1, 0}, //
    {1, 0, 1, 1}, //
    {0, 1, 0, 1}, //
    {0, 0, 1, 0}, //
    {0, 0, 0, 1}, //
  };
  u8 result[7] = {};
  for(i32 r = 0; r < 7; r += 1) {
    for(i32 c = 0; c < 4; c += 1) {
      result[r] += G[r][c] * vector[c];
    }
    result[r] %= 2;
  }
  u8 byte = 0;
  u8 parity = 0;
  for(i32 i = 0; i < 7; i += 1) {
    byte |= result[i] << i;
    parity += result[i];
  }
  byte |= (parity & 0x01) << 7;
  return byte;
}

[[nodiscard]] Hamminged hammingify(u8 const byte)
{
  u8 const d1 = byte & 0x01;
  u8 const d2 = (byte >> 1) & 0x01;
  u8 const d3 = (byte >> 2) & 0x01;
  u8 const d4 = (byte >> 3) & 0x01;
  u8 const d5 = (byte >> 4) & 0x01;
  u8 const d6 = (byte >> 5) & 0x01;
  u8 const d7 = (byte >> 6) & 0x01;
  u8 const d8 = (byte >> 7) & 0x01;

  Hamminged result = {hamming_byte(d4, d3, d2, d1),
                      hamming_byte(d8, d7, d6, d5)};
  return result;
}

i32 main(i32 const argc, char** const argv)
{
  STDOUT_Stream stdout;
  STDERR_Stream stderr;
  if(argc < 3) {
    anton::String_View executable(argv[0]);
    stderr.write(anton::format("{} <infile> <outfile>\n"_sv, executable));
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
  Array<u8> data{reserve, infile_size};
  data.force_size(infile_size);
  infile.read(data.data(), infile_size);
  infile.close();

  Array<u8> coded{reserve, data.size() * 2};
  coded.force_size(data.size() * 2);
  {
    u8 const* src = data.begin();
    u8 const* const end = data.end();
    u8* dst = coded.begin();
    for(; src != end; src += 1, dst += 2) {
      Hamminged h = hammingify(*src);
      *dst = h.first;
      *(dst + 1) = h.second;
    }
  }

  anton::String const outfilename(argv[2]);
  anton::fs::Output_File_Stream outfile(outfilename);
  if(!outfile.is_open()) {
    stderr.write(
      anton::format("error: could not open outfile '{}'\n"_sv, outfilename));
    return 1;
  }

  outfile.write(coded.data(), coded.size_bytes());
  outfile.close();

  return 0;
}
