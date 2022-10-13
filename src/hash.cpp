#include "hash.hpp"

#include <array>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "smallsha1/sha1.hpp"
#include "spdlog/spdlog.h"

namespace fur::hash {

std::string error_to_string(const HashError error) {
  switch (error) {
    case HashError::MalformedPieceHashesString:
      return "malformed piece hashes string";
    default:
      return "unknown error";
  }
}

std::string hash_to_str(const hash_t& hash) {
  return std::string{hash.begin(), hash.end()};
}

std::string hash_to_hex(const hash_t& hash) {
  char result[41];
  sha1::toHexString(hash.begin(), result);
  return std::string{result, 40};
}

hash_t compute_info_hash(const std::string& bencoded_info_dict) {
  hash_t buffer;
  sha1::calc(bencoded_info_dict.data(), bencoded_info_dict.length(),
             buffer.begin());
  return buffer;
}

auto split_piece_hashes(const std::string& piece_hashes_str) -> HashResult {
  if (piece_hashes_str.length() % 20 > 0) {
    return HashResult::ERROR(HashError::MalformedPieceHashesString);
  }

  std::vector<hash_t> result;
  // Forced to use unsigned because that's what "std::string::length" returns
  // even though signed integers are generally the better idea for iterator
  // variables
  for (unsigned i = 0; i < piece_hashes_str.length(); i += 20) {
    hash_t this_hash;
    std::fill(this_hash.begin(), this_hash.end(), 0);

    // "std::string" a vector of "char", not "unsigned char" like our "hash_t"
    // is. But we know we're forcing the compiler to make "char" unsigned so
    // this is actually safe. We just need to convert the type of "hash_t" from
    // an array of "unsigned char" to an array of "char" and we do that with
    // "reinterpret_cast"
    piece_hashes_str.copy(
        reinterpret_cast<std::array<char, 20>::iterator>(this_hash.begin()), 20,
        i);
    result.push_back(this_hash);
  }
  return HashResult::OK(std::move(result));
}

bool verify_piece(const std::vector<uint8_t>& piece, hash_t hash) {
  assert(piece.size() < std::numeric_limits<size_t>::max());
  hash_t buffer;
  sha1::calc(piece.data(), static_cast<int>(piece.size()), buffer.begin());
  return buffer == hash;
}

}  // namespace fur::hash
