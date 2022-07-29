#pragma once

#include <cstdint>
#include <string>

/// Type for a SHA1 hash
using hash_t = uint8_t[20];

std::string hash_to_str(const hash_t& hash);
