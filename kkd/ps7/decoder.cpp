#include <anton/console.hpp>
#include <anton/filesystem.hpp>
#include <anton/format.hpp>
#include <anton/iterators/range.hpp>
#include <anton/iterators/zip.hpp>

#include <types.hpp>

struct Unhamming_Result {
  u8 byte = 0;
  u8 errors = 0;
};

struct Hamming_Byte {
  u8 bits[7];
  u8 parity;
};

[[nodiscard]] Hamming_Byte decompose_hamming(u8 const byte)
{
  Hamming_Byte const result{
    .bits =
      {
        [0] = static_cast<u8>(byte & 0x01),
        [1] = static_cast<u8>(byte >> 1 & 0x01),
        [2] = static_cast<u8>(byte >> 2 & 0x01),
        [3] = static_cast<u8>(byte >> 3 & 0x01),
        [4] = static_cast<u8>(byte >> 4 & 0x01),
        [5] = static_cast<u8>(byte >> 5 & 0x01),
        [6] = static_cast<u8>(byte >> 6 & 0x01),
      },
    .parity = static_cast<u8>(byte >> 7 & 0x01),
  };
  return result;
};

enum struct Error_Kind {
  no_error,
  double_bit,
};

Error_Kind verify_hamming_byte(u8& out, Hamming_Byte byte)
{
  // Recalculate parities.
  u8 const parity = (byte.bits[0] + byte.bits[1] + byte.bits[2] + byte.bits[3] +
                     byte.bits[4] + byte.bits[5] + byte.bits[6]) %
                    2;
  // The check matrix.
  u8 H[3][7] = {
    {0, 0, 1, 0, 1, 1, 1},
    {0, 1, 0, 1, 1, 1, 0},
    {1, 0, 1, 1, 1, 0, 0},
  };
  u8 const* vector = byte.bits;
  u8 result[3] = {};
  for(i32 r = 0; r < 3; r += 1) {
    for(i32 c = 0; c < 7; c += 1) {
      result[r] += H[r][c] * vector[c];
    }
    result[r] %= 2;
  }
  u8 const syndrome = result[0] << 2 | result[1] << 1 | result[2];
  if(parity != byte.parity) {
    if(syndrome != 0) {
      u8 const map[8] = {0, 0, 1, 3, 6, 2, 5, 4};
      u8 const index = map[syndrome];
      byte.bits[index] = (byte.bits[index] + 1) % 2;
    }
    // Else the parity bit is faulty. We do not fix it because we are not
    // interested in it past this point.
  } else {
    if(syndrome != 0) {
      // Double bit error.
      memset(byte.bits, 0, sizeof(byte.bits));
      return Error_Kind::double_bit;
    }
  }

  u8 const b3 = (byte.bits[1] - byte.bits[0] + 2) % 2;
  out |= byte.bits[0] << 3 | b3 << 2 | byte.bits[5] << 1 | byte.bits[6];
  return Error_Kind::no_error;
}

[[nodiscard]] Unhamming_Result unhammingify(u8 const b1, u8 const b2)
{
  Hamming_Byte const byte1 = decompose_hamming(b1);
  Hamming_Byte const byte2 = decompose_hamming(b2);
  Unhamming_Result result;
  Error_Kind const err2 = verify_hamming_byte(result.byte, byte2);
  result.errors += err2 == Error_Kind::double_bit;
  result.byte <<= 4;
  Error_Kind const err1 = verify_hamming_byte(result.byte, byte1);
  result.errors += err1 == Error_Kind::double_bit;
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
  Array<u8> coded_data{reserve, infile_size};
  coded_data.force_size(infile_size);
  infile.read(coded_data.data(), infile_size);
  infile.close();

  Array<u8> data{reserve, coded_data.size() / 2};
  data.force_size(coded_data.size() / 2);
  i64 total_errors = 0;
  {
    u8 const* src = coded_data.begin();
    u8 const* const end = coded_data.end();
    u8* dst = data.begin();
    for(; src != end; src += 2, dst += 1) {
      u8 const b1 = *src;
      u8 const b2 = *(src + 1);
      Unhamming_Result const result = unhammingify(b1, b2);
      *dst = result.byte;
      total_errors += result.errors;
    }
  }

  anton::String const outfilename(argv[2]);
  anton::fs::Output_File_Stream outfile(outfilename);
  if(!outfile.is_open()) {
    stderr.write(
      anton::format("error: could not open outfile '{}'\n"_sv, outfilename));
    return 1;
  }
  outfile.write(data.data(), data.size_bytes());
  outfile.close();

  stdout.write(anton::format("double bit errors: {}\n", total_errors));

  return 0;
}
