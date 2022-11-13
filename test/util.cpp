#include "download/util.hpp"

#include <array>
#include <cstdint>

#include "catch2/catch.hpp"

TEST_CASE("[Util] Encode uint32") {
  REQUIRE(encode_integer(1) == std::array<uint8_t, 4>{0, 0, 0, 1});
  REQUIRE(encode_integer(2147483648) == std::array<uint8_t, 4>{0x80, 0, 0, 0});
}

TEST_CASE("[Util] Decode uint32") {
  REQUIRE(decode_integer(std::array<uint8_t, 4>{0, 0, 0, 1}) == 1);
  REQUIRE(decode_integer(std::array<uint8_t, 4>{0x80, 0, 0, 0}) == 2147483648);
}
