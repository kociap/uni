#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/optional.hpp>

#include <AAC.hpp>

using namespace anton::literals;

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

  decode_AAC(infile, outfile);

  return 0;
}
