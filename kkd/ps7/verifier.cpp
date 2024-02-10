#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/iterators/range.hpp>
#include <anton/iterators/zip.hpp>

#include <types.hpp>

i32 main(i32 const argc, char** const argv)
{
  STDOUT_Stream stdout;
  STDERR_Stream stderr;
  if(argc < 3) {
    anton::String_View executable(argv[0]);
    stderr.write(anton::format("{} <infile1> <infile2>\n"_sv, executable));
    return 1;
  }

  anton::String const infile1name(argv[1]);
  anton::fs::Input_File_Stream infile1(infile1name);
  if(!infile1.is_open() || infile1.error()) {
    stderr.write(
      anton::format("error: could not open infile1 '{}'\n"_sv, infile1name));
    return 1;
  }

  infile1.seek(Seek_Dir::end, 0);
  i64 const infile1_size = infile1.tell();
  infile1.seek(Seek_Dir::beg, 0);
  Array<u8> data1{reserve, infile1_size};
  data1.force_size(infile1_size);
  infile1.read(data1.data(), infile1_size);
  infile1.close();

  anton::String const infile2name(argv[2]);
  anton::fs::Input_File_Stream infile2(infile2name);
  if(!infile2.is_open() || infile2.error()) {
    stderr.write(
      anton::format("error: could not open infile2 '{}'\n"_sv, infile2name));
    return 1;
  }

  infile2.seek(Seek_Dir::end, 0);
  i64 const infile2_size = infile2.tell();
  infile2.seek(Seek_Dir::beg, 0);
  Array<u8> data2{reserve, infile2_size};
  data2.force_size(infile2_size);
  infile2.read(data2.data(), infile2_size);
  infile2.close();

  u8 const* src1 = data1.begin();
  u8 const* const end1 = data1.end();
  u8 const* src2 = data2.begin();
  u8 const* const end2 = data1.end();
  i64 blocks = 0;
  for(; src1 != end1 && src2 != end2; ++src1, ++src2) {
    u8 b1 = *src1;
    u8 b2 = *src2;
    if((b1 & 0xF0) != (b2 & 0xF0)) {
      blocks += 1;
    }

    if((b1 & 0x0F) != (b2 & 0x0F)) {
      blocks += 1;
    }
  }

  stdout.write(anton::format("incorrect blocks: {}\n"_sv, blocks));

  return 0;
}
