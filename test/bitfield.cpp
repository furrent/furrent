#include "download/bitfield.hpp"

#include <cstdint>
#include <sstream>
#include <vector>

#include "catch2/catch.hpp"
#include "tfriend.hpp"

using namespace fur::download::bitfield;

TEST_CASE("[Bitfield] Initialized to correct length") {
  Bitfield bf1(0);
  REQUIRE(TestingFriend::Bitfield_storage(bf1).empty());
  Bitfield bf2(3);
  REQUIRE(TestingFriend::Bitfield_storage(bf2).size() == 1);
  Bitfield bf3(8);
  REQUIRE(TestingFriend::Bitfield_storage(bf3).size() == 1);
  Bitfield bf4(9);
  REQUIRE(TestingFriend::Bitfield_storage(bf4).size() == 2);
}

TEST_CASE("[Bitfield] Basic Set/Get") {
  Bitfield bf(10);
  REQUIRE(TestingFriend::Bitfield_storage(bf) == std::vector<uint8_t>{0, 0});
  for (int i = 0; i < 10; i++) {
    REQUIRE(!bf.get(i));
  }

  bf.set(0);
  REQUIRE(bf.get(0));

  REQUIRE(TestingFriend::Bitfield_storage(bf) == std::vector<uint8_t>{128, 0});

  REQUIRE(!bf.get(7));
  bf.set(7);
  REQUIRE(bf.get(7));

  REQUIRE(TestingFriend::Bitfield_storage(bf) ==
          std::vector<uint8_t>{128 | 1, 0});

  REQUIRE(!bf.get(8));
  bf.set(8);
  REQUIRE(bf.get(8));

  REQUIRE(TestingFriend::Bitfield_storage(bf) ==
          std::vector<uint8_t>{128 | 1, 128});

  REQUIRE(!bf.get(9));
  bf.set(9);
  REQUIRE(bf.get(9));

  REQUIRE(TestingFriend::Bitfield_storage(bf) ==
          std::vector<uint8_t>{128 | 1, 128 | 64});

  REQUIRE(!bf.get(5));
  bf.unset(5);
  REQUIRE(!bf.get(5));

  REQUIRE(TestingFriend::Bitfield_storage(bf) ==
          std::vector<uint8_t>{128 | 1, 128 | 64});

  REQUIRE(bf.get(0));
  bf.unset(0);
  REQUIRE(!bf.get(0));

  REQUIRE(TestingFriend::Bitfield_storage(bf) ==
          std::vector<uint8_t>{1, 128 | 64});

  REQUIRE_THROWS(bf.set(10));
}

TEST_CASE("[Bitfield] Existing storage") {
  // 00000011 1000001-
  Bitfield bf{std::vector<uint8_t>{3, 130}, 15};
  REQUIRE(bf.get(6));
  REQUIRE(bf.get(7));
  REQUIRE(bf.get(8));
  REQUIRE(bf.get(14));
  REQUIRE_THROWS(bf.get(15));
}

TEST_CASE("[Bitfield] Print") {
  // 00000011 1000001-
  Bitfield bf{std::vector<uint8_t>{3, 130}, 15};
  std::stringstream ss;
  ss << bf;

  // Should be a grid like:
  // ......##
  // #.....#
  REQUIRE(ss.str() == "......##\n#.....#\n");
}

TEST_CASE("[Bitfield] Get bytes") {
  std::vector<uint8_t> bytes{128, 3};
  REQUIRE(bytes == Bitfield(bytes, 16).get_bytes());
}
