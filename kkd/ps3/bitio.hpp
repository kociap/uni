#pragma once

#include <anton/stream.hpp>

#include <types.hpp>

template<u64 N>
struct Bit_Stack {
public:
  void reset()
  {
    cur_byte = 0;
    cur_bit = 0;
  }

  void push(u8 const bit)
  {
    if(cur_bit == 8) {
      cur_bit = 0;
      cur_byte += 1;
    }

    u8& byte = bytes[cur_byte];
    byte = byte | ((bit & 0x1) << cur_bit);
    cur_bit += 1;
  }

  u8 pop()
  {
    ANTON_ASSERT(cur_byte > 0 || cur_byte == 0 && cur_bit > 0,
                 "pop called on empty stack");

    u8 const& byte = bytes[cur_byte];
    u8 const bit = (byte >> (cur_bit - 1)) & 0x1;
    cur_bit -= 1;
    if(cur_bit == 0) {
      cur_bit = 8;
      cur_byte -= 1;
    }

    return bit;
  }

  [[nodiscard]] i64 size() const
  {
    return cur_byte * 8 + cur_bit;
  }

private:
  u8 bytes[N / 8] = {};
  i64 cur_byte = 0;
  i64 cur_bit = 0;
};

struct Bit_Writer {
public:
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
