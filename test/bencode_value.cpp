#include "bencode_value.hpp"

#include "catch2/catch.hpp"
using namespace fur::bencode;

// ================
// BencodeInt
// ================
TEST_CASE("[BencodeValue::BencodeInt] Correctly decodes an encoded integer") {
  BencodeInt b_1(42);
  BencodeInt b_2(-42);
  REQUIRE(b_1.to_string() == "i42e");
  REQUIRE(b_1.value() == 42);
  REQUIRE(b_2.to_string() == "i-42e");
  REQUIRE(b_2.value() == -42);
  REQUIRE(b_1.value() + b_2.value() == 0);
}

// ================
// BencodeInt
// ================
TEST_CASE("[BencodeValue::BencodeString] Correctly decodes an encoded string") {
  BencodeString b_1("foo");
  REQUIRE(b_1.value() == "foo");
  REQUIRE(b_1.to_string() == "3:foo");
}

// ================
// BencodeList
// ================
TEST_CASE("[BencodeValue::BencodeList] Correctly decodes an encoded list") {
  std::vector<std::unique_ptr<BencodeValue>> v;
  v.push_back(std::make_unique<BencodeInt>(1));
  v.push_back(std::make_unique<BencodeString>("spam"));
  BencodeList l = BencodeList(std::move(v));
  REQUIRE(l.to_string() == "li1e4:spame");
}