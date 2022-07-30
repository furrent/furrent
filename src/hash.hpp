#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

/// Type for a SHA1 hash
using hash_t = std::array<uint8_t, 20>;

/// Encodes an hash to a string. Note that we're forcing the compiler to make
/// "char" unsigned so a string can contain arbitrary bytes
std::string hash_to_str(const hash_t& hash);

std::string hex(const hash_t& hash);

/// Takes a string with the hashes of pieces from a torrent file and parses
/// them into a vector of "hash_t". Each hash is 20 bytes long
std::vector<hash_t> split_piece_hashes(const std::string& pieces);
