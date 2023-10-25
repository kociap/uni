#pragma once

#include <anton/filesystem.hpp>

#include <bitio.hpp>
#include <types.hpp>

struct Encoding_Statistics {
  i64 frequencies[256];
  i64 read;
  i64 written;
};

using universal_encode_fn = void (*)(Bit_Writer& writer, u64 value);
using universal_decode_fn = u64 (*)(Bit_Reader& reader);

[[nodiscard]] Encoding_Statistics
compress_LZW(anton::fs::Input_File_Stream& input, anton::Output_Stream& output,
             universal_encode_fn encode);
void decompress_LZW(anton::fs::Input_File_Stream& input,
                    anton::Output_Stream& output, universal_decode_fn decode);
