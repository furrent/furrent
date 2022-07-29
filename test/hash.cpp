#include "hash.hpp"

#include <string>

#include "catch2/catch.hpp"

static void check_string(const std::string& string, const hash_t& hash) {
  REQUIRE(string.length() == 20);
  for (int i = 0; i < 10; i++) {
    REQUIRE(string[i] == (char)hash[i]);
  }
}

TEST_CASE("[Hash] Convert to string") {
  hash_t debian_info_hash = {0xcc, 0x5b, 0xf7, 0x2c, 0x0d, 0xb8, 0x4e,
                             0x2d, 0xe9, 0x5f, 0x96, 0x79, 0x54, 0x44,
                             0x1c, 0x01, 0x7c, 0x5a, 0x36, 0x31};
  auto string = hash_to_str(debian_info_hash);
  check_string(string, debian_info_hash);
}

TEST_CASE("[Hash] Convert to string (With null character inside)") {
  hash_t not_quite_debian_info_hash = {0xcc, 0x5b, 0xf7, 0x2c, 0x0d, 0xb8, 0x4e,
                                       0x2d, 0xe9, 0x5f, 0x00, 0x79, 0x54, 0x44,
                                       0x1c, 0x01, 0x7c, 0x5a, 0x36, 0x31};
  auto string = hash_to_str(not_quite_debian_info_hash);
  check_string(string, not_quite_debian_info_hash);
}
