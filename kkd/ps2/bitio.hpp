#pragma once

#include <anton/stream.hpp>

#include <types.hpp>

struct Bit_Writer {
  void reset(anton::Output_Stream* stream)
  {
    this->stream = stream;
    bits = 0;
    byte = 0;
    written = 0;
  }

  void flush()
  {
    if(bits == 0) {
      return;
    }

    byte <<= (8 - bits);
    stream->put(byte);
    written += 1;
  }

  // write
  //
  // Parameters:
  // bit - the bit must be at LSB. All other bits must be 0.
  //
  void write(u8 const bit)
  {
    byte = (byte << 1) | bit;
    bits += 1;
    if(bits == 8) {
      stream->put(byte);
      byte = 0;
      bits = 0;
      written += 1;
    }
  }

  [[nodiscard]] i64 get_written()
  {
    return written;
  }

private:
  anton::Output_Stream* stream = nullptr;
  u8 byte = 0;
  i32 bits = 0;
  i64 written = 0;
};

struct Bit_Reader {
  void reset(anton::Input_Stream* stream)
  {
    this->stream = stream;
    bits = 0;
    byte = 0;
  }

  [[nodiscard]] u8 read()
  {
    if(bits == 0) {
      byte = stream->get();
      bits = 8;
    }

    u8 const value = (byte >> (bits - 1)) & 0x1;
    bits -= 1;
    return value;
  }

private:
  anton::Input_Stream* stream = nullptr;
  u8 byte = 0;
  i32 bits = 0;
};
