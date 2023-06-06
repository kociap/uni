#pragma once

#include <types.hpp>

#include <vector>

// encode_frames
//
// Parameters:
//      begin
//        end
// frame_size - maximum size of a single frame in bytes.
//
[[nodiscard]] std::vector<u8> encode_frames(u8 const* begin, u8 const* end,
                                            i32 frame_size = 32);

[[nodiscard]] std::vector<u8> decode_frames(u8 const* begin, u8 const* end);
