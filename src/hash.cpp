#include "hash.hpp"

// TODO Add test
std::string hash_to_str(const hash_t& hash) {
  // "static_cast" cannot be used because the compiler cannot infer that
  // "char" is the same as "unsigned char" even though we forced that with a
  // flag. Must use "reinterpret_cast". The second parameter lets us include
  // any null bytes.
  return std::string{reinterpret_cast<const char*>(hash), 20};
}
