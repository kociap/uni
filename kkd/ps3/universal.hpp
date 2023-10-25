#pragma once

#include <bitio.hpp>

void encode_elias_omega(Bit_Writer& writer, u64 value);
[[nodiscard]] u64 decode_elias_omega(Bit_Reader& reader);

void encode_elias_delta(Bit_Writer& writer, u64 value);
[[nodiscard]] u64 decode_elias_delta(Bit_Reader& reader);

void encode_elias_gamma(Bit_Writer& writer, u64 value);
[[nodiscard]] u64 decode_elias_gamma(Bit_Reader& reader);

void encode_fibonacci(Bit_Writer& writer, u64 value);
[[nodiscard]] u64 decode_fibonacci(Bit_Reader& reader);
