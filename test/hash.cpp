#include "hash.hpp"

#include <cstdint>
#include <string>
#include <vector>

#include "catch2/catch.hpp"
#include "smallsha1/sha1.hpp"

using namespace fur::hash;

static void check_string(const std::string& string, const hash_t& hash) {
  REQUIRE(string.length() == 20);
  for (int i = 0; i < 10; i++) {
    REQUIRE(static_cast<uint8_t>(string[i]) == hash[i]);
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

TEST_CASE("[Hash] Split pieces string") {
  char piece_hashes[] =
      "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
      "\x00\x00"
      "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
      "\x01\x01"
      "\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02\x02"
      "\x02\x02";
  std::vector<hash_t> pieces =
      split_piece_hashes(std::string{piece_hashes, 3 * 20});
  REQUIRE(pieces.size() == 3);
  for (int i = 0; i < 3; i++) {
    hash_t want;
    std::fill(want.begin(), want.end(), i);
    REQUIRE(pieces[i] == want);
  }
}

TEST_CASE("[Hash] SHA1 dependency is correct") {
  hash_t hash;
  std::fill(hash.begin(), hash.end(), 0);

  unsigned char hash_this[] = "Furrent is a neat BitTorrent client with fur";
  sha1::calc(hash_this, sizeof(hash_this) - 1, hash.begin());

  REQUIRE(hex(hash) == "3a773b8553a663941552a0df3b5968b4695cb212");
}
