#pragma once

#include <anton/filesystem.hpp>

#include <types.hpp>

struct Encoding_Statistics {
  i64 frequencies[256];
  i64 read;
  i64 written;
};

[[nodiscard]] Encoding_Statistics
encode_AAC(anton::fs::Input_File_Stream& input, anton::Output_Stream& output);
void decode_AAC(anton::fs::Input_File_Stream& input,
                anton::Output_Stream& output);
