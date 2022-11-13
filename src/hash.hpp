#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "util/result.hpp"

namespace fur::hash {

/// Type for a SHA1 hash
using hash_t = std::array<uint8_t, 20>;

enum class HashError {
  MalformedPieceHashesString,
  PieceHashesStringTooLarge,
};

/// Function to translate an HashError to a string
std::string error_to_string(HashError error);

/// Encodes an hash to a string. Note that we're forcing the compiler to make
/// "char" unsigned so a string can contain arbitrary bytes
std::string hash_to_str(const hash_t& hash);

/// Returns an hex string for the provided hash
std::string hash_to_hex(const hash_t& hash);

/// Computes the info hash given a bencoded string for the .torrent info dict
hash_t compute_info_hash(const std::string& bencoded_info_dict);

/// Takes a string with the hashes of pieces from a torrent file and parses
/// them into a vector of "hash_t". Each hash is 20 bytes long
util::Result<std::vector<hash_t>, HashError> split_piece_hashes(
    const std::string& pieces);

/// Checks that a downloaded piece matches the provided hash
bool verify_piece(const std::vector<uint8_t>& piece, hash_t hash);
}  // namespace fur::hash
