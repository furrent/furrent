//
// Created by nicof on 28/07/22.
//
#include "bencode_value.hpp"

#include "catch2/catch.hpp"
using namespace bencode;

// ================
// BencodeInt
// ================
TEST_CASE("[BencodeInt] Correctly decodes an encoded integer") {
  BencodeInt b_1("i42e");
  BencodeInt b_2("i-42e");
  BencodeInt b_3("i0e");
  REQUIRE(b_1.value() == 42);
  REQUIRE(b_2.value() == -42);
  REQUIRE(b_3.value() == 0);
}

TEST_CASE("[BencodeInt] Wrongly decodes an encoded integer") {
  REQUIRE_THROWS_AS(BencodeInt("i42"), std::invalid_argument);
  REQUIRE_THROWS_AS(BencodeInt("i-42"), std::invalid_argument);
  REQUIRE_THROWS_AS(BencodeInt("i-0"), std::invalid_argument);
}

TEST_CASE("[BencodeString] Correctly decodes an encoded string") {
  BencodeString b_1("4:spam");
  BencodeString b_2("3:foo");
  REQUIRE(b_1.value() == "spam");
  REQUIRE(b_2.value() == "foo");
}

TEST_CASE("[BencodeString] Wrongly decodes an encoded string") {
  REQUIRE_THROWS_AS(BencodeString("3:spam"), std::invalid_argument);
  REQUIRE_THROWS_AS(BencodeString("3x:foo"), std::invalid_argument);
}