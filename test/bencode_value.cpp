//
// Created by nicof on 28/07/22.
//
#include "catch2/catch.hpp"
#include "bencode_value.h"
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
