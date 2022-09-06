#pragma once

#include <array>
#include <cstdint>

/// Takes a 32 bits unsigned integer and encodes it into a 4 bytes array.
std::array<uint8_t, 4> encode_big_endian(uint32_t n);

/// Decodes a 32 bits unsigned integer from a 4 bytes array.
uint32_t decode_big_endian(const std::array<uint8_t, 4>& buf);
