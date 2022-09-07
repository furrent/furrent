#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "util/result.hpp"

namespace fur::hash {

/// Type for a SHA1 hash
using hash_t = std::array<uint8_t, 20>;

enum class HashError{
  // The hash of the given string is malformed
  MalformedPieceHashesString
};

/// Result of a hash operation
typedef util::Result<std::vector<hash_t>, HashError>
    HashResult;

/// Encodes an hash to a string. Note that we're forcing the compiler to make
/// "char" unsigned so a string can contain arbitrary bytes
std::string hash_to_str(const hash_t& hash);

/// Returns an hex string for the provided hash
std::string hash_to_hex(const hash_t& hash);

/// Computes the info hash given a bencoded string for the .torrent info dict
hash_t compute_info_hash(const std::string& bencoded_info_dict);

/// Takes a string with the hashes of pieces from a torrent file and parses
/// them into a vector of "hash_t". Each hash is 20 bytes long
HashResult split_piece_hashes(const std::string& pieces);
}  // namespace fur::hash
