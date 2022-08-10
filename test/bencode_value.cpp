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
  REQUIRE(b_1.get_type() == BencodeType::Integer);
  REQUIRE(b_2.to_string() == "i-42e");
  REQUIRE(b_2.value() == -42);
  REQUIRE(b_2.get_type() == BencodeType::Integer);
  REQUIRE(b_1.value() + b_2.value() == 0);
}

// ================
// BencodeInt
// ================
TEST_CASE("[BencodeValue::BencodeString] Correctly decodes an encoded string") {
  BencodeString b_1("foo");
  REQUIRE(b_1.value() == "foo");
  REQUIRE(b_1.to_string() == "3:foo");
  REQUIRE(b_1.get_type() == BencodeType::String);
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
  REQUIRE(l.get_type() == BencodeType::List);
}

// ================
// BencodeDict
// ================
TEST_CASE("[BencodeValue::BencodeDict] Correctly decodes an encoded dict") {
  std::map<std::string, std::unique_ptr<BencodeValue>> m;
  m.insert({"foo", std::make_unique<BencodeInt>(1)});
  m.insert({"bar", std::make_unique<BencodeString>("spam")});
  BencodeDict d = BencodeDict(m);
  REQUIRE(d.to_string() == "d3:fooi1e4:bar4:spame");
  REQUIRE(d.get_type() == BencodeType::Dict);
}