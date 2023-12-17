#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/iterators/range.hpp>
#include <anton/iterators/zip.hpp>

#include <tga.hpp>
#include <types.hpp>

[[nodiscard]] static Array<Pixel> unfilter(Slice<Pixel const> const low,
                                           Slice<Pixel const> const high)
{
  STDOUT_Stream stdout;
  Array<Pixel> unfiltered{reserve, low.size()};
  for(auto [lp, hp]: Range(Zip_Iterator(low.begin(), high.begin()),
                           Zip_Iterator(low.end(), high.end()))) {
    Pixel const p1 = hp - lp; // 1/2 p - 1/2 prev + 1/2 p + 1/2 prev
    Pixel const p2 = hp + lp; // 1/2 p - 1/2 prev + 1/2 p + 1/2 prev
    unfiltered.push_back(p1.normalised());
    unfiltered.push_back(p2.normalised());
  }
  return unfiltered;
}

[[nodiscard]] static Array<Pixel> undifference(Slice<Pixel const> const data)
{
  Array<Pixel> pixels{reserve, data.size()};
  pixels.force_size(data.size());
  pixels[0] = data[0];
  Pixel const* current = data.begin() + 1;
  Pixel const* const end = data.end();
  Pixel const* prev = pixels.begin();
  Pixel* dst = pixels.data() + 1;
  while(current != end) {
    *dst = *current + *prev;
    ++current;
    ++prev;
    ++dst;
  }
  return pixels;
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
  Array<u8> coded_data{reserve, infile_size};
  coded_data.force_size(infile_size);
  infile.read(coded_data.data(), infile_size);
  infile.close();

  Custom_Header header = *reinterpret_cast<Custom_Header*>(coded_data.data());
  Pixel const* const low_section_begin =
    reinterpret_cast<Pixel const*>(coded_data.data() + sizeof(Custom_Header));
  Pixel const* const high_section_begin = low_section_begin + header.low_size;
  Slice<Pixel const> differenced_low(low_section_begin, header.low_size);
  Slice<Pixel const> high(high_section_begin, header.high_size);

  Array<Pixel> low = undifference(differenced_low);
  Array<Pixel> pixels = unfilter(low, high);

  // TGA kinda dumb in that it stores pixels in little-endian (BGR instead of
  // RGB), hence we have to swap the bytes around.
  for(Pixel& p: pixels) {
    swap(p.r, p.b);
  }

  Array<u8> color_data{reserve, pixels.size() * 3};
  color_data.force_size(pixels.size() * 3);
  {
    pixel_element_t const* src =
      reinterpret_cast<pixel_element_t const*>(pixels.begin());
    pixel_element_t const* end =
      reinterpret_cast<pixel_element_t const*>(pixels.end());
    copy(src, end, color_data.data());
  }
  anton::String const outfilename(argv[2]);
  anton::fs::Output_File_Stream outfile(outfilename);
  if(!outfile.is_open()) {
    stderr.write(
      anton::format("error: could not open outfile '{}'\n"_sv, outfilename));
    return 1;
  }
  outfile.write(header.tga_header, 18);
  outfile.write(color_data.data(), color_data.size_bytes());
  outfile.close();

  return 0;
}
