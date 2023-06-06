#include <utility.hpp>

// u16 crc16_ccitt(u8 const* begin, u8 const* const end) {
//   u16 crc = 0;
//   while(begin != end) {
//     // crc = (u8)(crc >> 8) | (crc << 8);
//     // crc ^= *begin;
//     // crc ^= (u8)(crc & 0xFF) >> 4;
//     // crc ^= (crc << 8) << 4;
//     // crc ^= ((crc & 0xFF) << 4) << 1;
//     u16 t = crc ^ *begin;
//     t = (t ^ t << 4) & 0xFF;
//     crc = (crc >> 8) ^ (t << 8) ^ (t >> 4) ^ (t << 3);
//     ++begin;
//   }
//   return crc;
// }

u16 crc16_ccitt(u8 const* begin, u8 const* const end) {
  unsigned short crc = 0xFFFF;

  for(; begin != end; ++begin) {
    crc ^= (*begin << 8);

    for(i32 j = 0; j < 8; ++j) {
      if(crc & 0x8000) {
        crc = (crc << 1) ^ 0x1021;
      } else {
        crc <<= 1;
      }
    }
  }

  return crc;
}
