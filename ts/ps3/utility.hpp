#pragma once

#include <types.hpp>

#include <vector>

struct Byte_Packer {
private:
  std::vector<u8> bytes;
  i32 packed_bits = 0;
  u8 byte = 0;

public:
  void append_bit(bool const bit) {
    byte = (byte << 1) | bit;
    packed_bits += 1;
    if(packed_bits == 8) {
      bytes.push_back(byte);
      byte = 0;
      packed_bits = 0;
    }
  }

  void append_byte(u8 const byte) {
    for(i32 i = 7; i >= 0; i -= 1) {
      bool const bit = (byte >> i) & 1;
      append_bit(bit);
    }
  }

  std::vector<u8> get_bytes() {
    if(packed_bits != 0) {
      byte <<= 8 - packed_bits;
      bytes.push_back(byte);
    }
    packed_bits = 0;
    byte = 0;
    return std::move(bytes);
  }
};