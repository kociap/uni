#include <frame.hpp>

#include <utility.hpp>

#include <fstream>
#include <iostream>
#include <optional>
#include <string>

std::optional<std::vector<u8>> read_byte_file(std::string const& path) {
  std::ifstream file(path);
  if(!file.is_open()) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return std::nullopt;
  }

  Byte_Packer packer;
  char bit_character;
  while(file.get(bit_character)) {
    bool const bit = bit_character == '1';
    packer.append_bit(bit);
  }

  file.close();

  return packer.get_bytes();
}

void write_byte_file(std::string const& path, u8 const* begin,
                     u8 const* const end) {
  std::ofstream file(path);
  if(!file.is_open()) {
    std::cerr << "Failed to open file: " << path << std::endl;
    return;
  }

  for(; begin != end; ++begin) {
    u8 const byte = *begin;
    for(i32 i = 7; i >= 0; i -= 1) {
      bool const bit = (byte >> i) & 1;
      if(bit) {
        file.put('1');
      } else {
        file.put('0');
      }
    }
  }

  file.close();
}

int main() {
  constexpr i32 RETURN_FAILURE = 1;
  constexpr i32 RETURN_SUCCESS = 0;
#if defined(FRAME_ENCODE)
  std::optional<std::vector<u8>> result = read_byte_file("input.txt");
  if(!result) {
    return RETURN_FAILURE;
  }

  std::vector<u8> encoded_frames =
    encode_frames(result->data(), result->data() + result->size(), 2);
  write_byte_file("frame.txt", encoded_frames.data(),
                  encoded_frames.data() + encoded_frames.size());
  return RETURN_SUCCESS;
#elif defined(FRAME_DECODE)
  std::optional<std::vector<u8>> result = read_byte_file("frame.txt");
  if(!result) {
    return RETURN_FAILURE;
  }

  std::vector<u8> decoded_frames =
    decode_frames(result->data(), result->data() + result->size());
  write_byte_file("output.txt", decoded_frames.data(),
                  decoded_frames.data() + decoded_frames.size());
  return RETURN_SUCCESS;
#else
  #error "neither encode nor decode"
#endif
}
