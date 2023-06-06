#include <frame.hpp>

#include <crc.hpp>
#include <utility.hpp>

#include <optional>

static constexpr u8 TOKEN_SOT = 0x7E;
static constexpr u8 TOKEN_EOT = 0x7E;

static void encode_frame(Byte_Packer& packer, u8 const* begin,
                         u8 const* const end) {
  u16 const crc = crc16_ccitt(begin, end);
  // Pack SOT.
  packer.append_byte(TOKEN_SOT);
  // Bitstuff and pack data.
  i32 consecutive_ones = 0;
  for(; begin != end; ++begin) {
    u8 const byte = *begin;

    for(i32 i = 7; i >= 0; i -= 1) {
      bool const bit = (byte >> i) & 1;

      if(!bit) {
        consecutive_ones = 0;
      } else {
        consecutive_ones += 1;
      }

      packer.append_bit(bit);
      if(consecutive_ones == 5) {
        packer.append_bit(0);
        consecutive_ones = 0;
      }
    }
  }

  // Bitstuff and pack crc.
  for(i32 i = 15; i >= 0; i -= 1) {
    bool const bit = (crc >> i) & 1;

    if(!bit) {
      consecutive_ones = 0;
    } else {
      consecutive_ones += 1;
    }

    packer.append_bit(bit);
    if(consecutive_ones == 5) {
      packer.append_bit(0);
      consecutive_ones = 0;
    }
  }

  // Pack EOT.
  packer.append_byte(TOKEN_EOT);
}

std::vector<u8> encode_frames(u8 const* begin, u8 const* const end,
                              i32 const frame_size) {
  Byte_Packer packer;
  i32 bytes = end - begin;
  while(bytes > 0) {
    i32 const size = bytes > frame_size ? frame_size : bytes;
    bytes -= size;
    encode_frame(packer, begin, begin + size);
    begin += size;
  }
  return packer.get_bytes();
}

[[nodiscard]] static std::optional<std::vector<u8>>
decode_frame(u8 const* begin, i32 bit_length, i32 const bit_offset) {
  Byte_Packer packer;

  i32 consecutive_ones = 0;
  {
    u8 const byte = *begin;
    ++begin;

    for(i32 i = 7 - bit_offset; i >= 0; i -= 1, bit_length -= 1) {
      bool const bit = (byte >> i) & 1;
      if(consecutive_ones == 5) {
        consecutive_ones = 0;
        continue;
      }

      packer.append_bit(bit);
      if(!bit) {
        consecutive_ones = 0;
      } else {
        consecutive_ones += 1;
      }
    }
  }

  for(; bit_length > 0; ++begin) {
    u8 const byte = *begin;
    for(i32 i = 7; i >= 0 && bit_length > 0; i -= 1, bit_length -= 1) {
      bool const bit = (byte >> i) & 1;
      if(consecutive_ones == 5) {
        consecutive_ones = 0;
        continue;
      }

      packer.append_bit(bit);
      if(!bit) {
        consecutive_ones = 0;
      } else {
        consecutive_ones += 1;
      }
    }
  }

  std::vector<u8> result = packer.get_bytes();
  if(result.size() < 2) {
    return std::nullopt;
  }

  u16 const crc = result[result.size() - 2] << 8 | result[result.size() - 1];
  // Pop crc from data.
  result.pop_back();
  result.pop_back();
  u16 const crc_check =
    crc16_ccitt(result.data(), result.data() + result.size());
  if(crc_check == crc) {
    return result;
  } else {
    return std::nullopt;
  }
}

[[nodiscard]] static i32 scan_token(u8 const* begin, u8 const* const end,
                                    u8 const token) {
  if(begin == end) {
    return -1;
  }

  i32 bit_offset = 0;
  u8 left_mask = 0xFF;
  u8 right_mask = 0x00;
  // A more optimal solution would be to shift the masks right instead of left.
  // That way we would avoid unnecessary shift operations.
  while(true) {
    if(right_mask == 0x00) {
      u8 const left_byte = *begin;
      if(left_byte == token) {
        return bit_offset;
      }
    } else {
      if(begin + 1 == end) {
        break;
      }

      u8 const left_byte = *begin;
      u8 const right_byte = *(begin + 1);
      // Modulo 8.
      i32 const mask_shift = bit_offset & 0x07;
      u8 const left_pattern = (left_byte & left_mask) << mask_shift;
      u8 const right_pattern = (right_byte & right_mask) >> (8 - mask_shift);
      u8 const pattern = left_pattern | right_pattern;
      if(pattern == token) {
        return bit_offset;
      }
    }
    // Shift bits through masks.
    left_mask >>= 1;
    right_mask = (right_mask >> 1) | 0x80;
    // Advance in the byte range.
    if(right_mask == 0xFF) {
      left_mask = 0xFF;
      right_mask = 0x00;
      begin += 1;
    }
    bit_offset += 1;
  }
  return -1;
}

std::vector<u8> decode_frames(u8 const* begin, u8 const* const end) {
  std::vector<u8> result;
  while(true) {
    i32 const sot_offset = scan_token(begin, end, TOKEN_SOT);
    if(sot_offset == -1) {
      break;
    }
    begin += sot_offset / 8;

    i32 const eot_offset = scan_token(begin + 1, end, TOKEN_EOT);
    if(eot_offset == -1) {
      break;
    }

    // sot_offset modulo 8.
    i32 const bit_offset = (sot_offset & 0x07);
    i32 const bit_length = eot_offset - bit_offset;
    std::optional<std::vector<u8>> frame =
      decode_frame(begin + 1, bit_length, bit_offset);
    if(frame) {
      result.insert(result.end(), frame->begin(), frame->end());
    }

    begin += eot_offset / 8 + 1;
  }
  return result;
}
