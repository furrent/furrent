#include <stdexcept>
#include <limits>

#include "download/util.hpp"

std::array<uint8_t, 4> encode_integer(int64_t n) {
  if (n < 0) {
    throw std::invalid_argument("expected positive integer");
  } else if (n > std::numeric_limits<uint32_t>::max()) {
    throw std::invalid_argument("integer is too big");
  }
  auto encode_me = static_cast<uint32_t>(n);

  std::array<uint8_t, 4> result{};
  for (int64_t i = 3; i >= 0; i--) {
    result[i] = encode_me & 0xFF;
    encode_me >>= 8;
  }
  return result;
}

int64_t decode_integer(const std::array<uint8_t, 4>& buf) {
  uint32_t result = 0;
  for (int64_t i = 0; i <= 3; i++) {
    result |= buf[i] << (8 * (3 - i));
  }
  // An uint32_t always fits inside a int64_t
  return static_cast<int64_t>(result);
}
