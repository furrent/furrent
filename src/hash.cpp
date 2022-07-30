#include "hash.hpp"

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

std::string hash_to_str(const hash_t& hash) {
  return std::string{hash.begin(), hash.end()};
}

std::vector<hash_t> split_piece_hashes(const std::string& piece_hashes_str) {
  if (piece_hashes_str.length() % 20 > 0) {
    throw std::invalid_argument("malformed piece hashes string");
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
  return result;
}
