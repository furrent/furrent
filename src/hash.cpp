#include "hash.hpp"

#include <array>
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
    case HashError::PieceHashesStringTooLarge:
      return "piece hashes string is too large";
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
  if (bencoded_info_dict.length() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    throw std::invalid_argument("input info dict is too large");
  }
  // smallsha1 uses int for the length
  int len = static_cast<int>(bencoded_info_dict.length());

  hash_t buffer;
  sha1::calc(bencoded_info_dict.data(), len, buffer.begin());
  return buffer;
}

util::Result<std::vector<hash_t>, HashError> split_piece_hashes(
    const std::string& piece_hashes_str) {
  using Result = util::Result<std::vector<hash_t>, HashError>;

  if (piece_hashes_str.length() > static_cast<size_t>(std::numeric_limits<int64_t>::max())) {
    return Result::ERROR(HashError::PieceHashesStringTooLarge);
  } else if (piece_hashes_str.length() % 20 > 0) {
    return Result::ERROR(HashError::MalformedPieceHashesString);
  }
  auto s_len = static_cast<int64_t>(piece_hashes_str.length());

  std::vector<hash_t> result;
  // Forced to use unsigned because that's what "std::string::length" returns
  // even though signed integers are generally the better idea for iterator
  // variables
  for (int64_t i = 0; i < s_len; i += 20) {
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
  return Result::OK(std::move(result));
}

bool verify_piece(const std::vector<uint8_t>& piece, hash_t hash) {
  if (piece.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    throw std::invalid_argument("piece is too big");
  }
  // smallsha1 uses int for the length
  int len = static_cast<int>(piece.size());

  hash_t buffer;
  sha1::calc(piece.data(), len, buffer.begin());
  return buffer == hash;
}

}  // namespace fur::hash
