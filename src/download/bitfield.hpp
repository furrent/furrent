#pragma once

#include <cstdint>
#include <ostream>
#include <vector>

#include "tfriend_fw.hpp"

namespace fur::download::bitfield {
/// Bitfield is basically just a bit array that is used to keep track of what
/// pieces a peer has to offer. A 1-bit indicates a piece that we can ask for
/// while a 0-bit is a piece that the peer doesn't have.
/// \attention A `Bitfield` cannot be resized once created.
///
/// Note that we could've used a type from the standard library instead but they
/// had a couple of disadvantages:
///   - `bitset` only works with a number of bits known at compile time but we
///     don't know the number of bits, which matches the number of pieces, until
///     we read a torrent file.
///   - `std::vector<bool>` is a specialized template which shouldn't actually
///     result in entire bytes being allocated. Because it is not, then, a
///     standard STL container, its usage is discouraged. See
///     https://en.wikipedia.org/wiki/Bit_array.
class Bitfield {
 public:
  /// Create a new `Bitfield` with the provided length (in bits).
  explicit Bitfield(int len);

  /// Create a new `Bitfield` from an existing backing storage. The length
  /// in bits must be provided because the last byte is not necessarily
  /// used till the last bit.
  Bitfield(std::vector<uint8_t> storage, int len)
      : len{len}, storage{std::move(storage)} {}

  /// Set the bit to the provided index to either 1 or 0, depending on `value`.
  void set(int index, bool value);

  /// Set the bit to the provided index to 1
  void set(int index) { set(index, true); }
  /// Set the bit to the provided index to 0
  void unset(int index) { set(index, false); }

  /// Get the value of the bit at the provided index.
  [[nodiscard]] bool get(int index) const;

  /// Display the `Bitfield` as a 8-column grid. Each row is a byte. Bit 0 is in
  /// the topmost left corner.
  friend std::ostream& operator<<(std::ostream& os, const Bitfield& bitfield);

  /// Store the length for checking that no out-of-bounds read or write is
  /// performed. The actual length in bits cannot be computed from the number of
  /// bytes in `storage` because the last byte might not be used in its
  /// entirety if the number of bits is not a multiple of 8.
  /// \attention A `Bitfield` cannot be resized once created.
  const int len;

 private:
  /// Backing bytes array.
  std::vector<uint8_t> storage;

  // Befriend this class so the unit tests are able to access private members.
  friend TestingFriend;
};
}  // namespace fur::download::bitfield
