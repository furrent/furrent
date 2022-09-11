#include "download/bitfield.hpp"

#include <stdexcept>

/// What byte contains the bit at `bit_index`?
uint32_t byte_index(uint32_t bit_index) { return bit_index / 8; }
/// How many bits from the beginning of the correct byte does the bit at
/// `bit_index` live?
uint32_t byte_offset(uint32_t bit_index) { return bit_index % 8; }

/// Throws an exception if `index` is not within the bounds of the bitfield
// Not using `Result` because this is truly exceptional behavior which shouldn't
// be handled in code.
void assert_within_bounds(uint32_t len, uint32_t index) {
  if (index >= len) {
    throw std::invalid_argument("bitfield is too short for this index");
  }
}

const uint8_t INDEX_ZERO = 0 | (1 << 7);

namespace fur::download::bitfield {
Bitfield::Bitfield(uint32_t len) : len{len} { storage.resize((len + 7) / 8); }

void Bitfield::set(uint32_t index, bool value) {
  assert_within_bounds(len, index);

  auto _byte_index = byte_index(index);
  if (value) {
    storage[_byte_index] =
        storage[_byte_index] | (INDEX_ZERO >> byte_offset(index));
  } else {
    storage[_byte_index] =
        storage[_byte_index] & ~(INDEX_ZERO >> byte_offset(index));
  }
}

bool Bitfield::get(uint32_t index) const {
  assert_within_bounds(len, index);

  auto _byte_index = byte_index(index);
  return storage[_byte_index] & (INDEX_ZERO >> byte_offset(index));
}

std::ostream& operator<<(std::ostream& os, const Bitfield& bf) {
  const char BIT_0 = '.';
  const char BIT_1 = '#';

  for (uint32_t i = 0; i < bf.len; i++) {
    if (bf.get(i)) {
      os << BIT_1;
    } else {
      os << BIT_0;
    }

    if ((i + 1) % 8 == 0) {
      os << '\n';
    }
  }
  os << '\n';
  return os;
}
}  // namespace fur::download::bitfield
