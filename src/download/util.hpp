#pragma once

#include <array>
#include <cstdint>

/// Takes an integer and encodes to a byte array. We take int64_t as input
/// because that's the integer type used throughout Furrent but the Torrent
/// protocol works with 32bits unsigned integer so we expect n to be greater
/// or equal than 0 and to fit in 32bits.
std::array<uint8_t, 4> encode_integer(int64_t n);

/// Decodes an integer from a byte array. We return an int64_t as output because
/// that's the integer type used throughout Furrent but the Torrent protocol
/// works with 32bits unsigned integers so that's what will be decoded. Because
/// any uint32_t can fit inside an int64_t, it's a safe cast to do.
int64_t decode_integer(const std::array<uint8_t, 4>& buf);
