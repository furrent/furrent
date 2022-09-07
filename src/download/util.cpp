#include "download/util.hpp"

std::array<uint8_t, 4> encode_big_endian(uint32_t n) {
  std::array<uint8_t, 4> result{};
  for (int i = 3; i >= 0; i--) {
    result[i] = n & 0xFF;
    n >>= 8;
  }
  return result;
}

uint32_t decode_big_endian(const std::array<uint8_t, 4>& buf) {
  uint32_t result = 0;
  for (int i = 0; i <= 3; i++) {
    result |= buf[i] << (8 * (3 - i));
  }
  return result;
}
