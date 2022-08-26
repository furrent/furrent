#include "download/bitfield.hpp"

#include <stdexcept>

/// What byte contains the bit at `bit_index`?
int byte_index(int bit_index) { return bit_index / 8; }
/// How many bits from the beginning of the correct byte does the bit at
/// `bit_index` live?
int byte_offset(int bit_index) { return bit_index % 8; }

/// Throws an exception if `index` is not within the bounds of the bitfield
void assert_within_bounds(int len, int index) {
  if (index >= len) {
    throw std::invalid_argument("bitfield is too short for this index");
  }
}

const uint8_t INDEX_ZERO = 0 | (1 << 7);

namespace fur::download::bitfield {
Bitfield::Bitfield(int len) : len{len} { storage.resize((len + 7) / 8); }

void Bitfield::set(int index, bool value) {
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

bool Bitfield::get(int index) const {
  assert_within_bounds(len, index);

  auto _byte_index = byte_index(index);
  return storage[_byte_index] & (INDEX_ZERO >> byte_offset(index));
}

std::ostream& operator<<(std::ostream& os, const Bitfield& bf) {
  const char BIT_0 = '.';
  const char BIT_1 = '#';

  for (int i = 0; i < bf.len; i++) {
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
