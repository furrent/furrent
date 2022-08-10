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
  // Checking correct list
  REQUIRE(l.to_string() == "li1e4:spame");
  REQUIRE(l.get_type() == BencodeType::List);
  // Checking correct value
  REQUIRE(l.value().size() == 2);
  REQUIRE(l.value()[0]->get_type() == BencodeType::Integer);
  REQUIRE(l.value()[1]->get_type() == BencodeType::String);

}

// ================
// BencodeDict
// ================
TEST_CASE("[BencodeValue::BencodeDict] Correctly decodes an encoded dict") {
  // Creating dict
  std::map<std::string, std::unique_ptr<BencodeValue>> m;
  m.insert({"bar", std::make_unique<BencodeString>("spam")});
  m.insert({"foo", std::make_unique<BencodeInt>(42)});
  BencodeDict d = BencodeDict(std::move(m));
  // Check correct dict
  REQUIRE(d.to_string() == "d3:bar4:spam3:fooi42ee");
  REQUIRE(d.get_type() == BencodeType::Dict);
  auto dict = &d.value();
  // Check correct dict values
  REQUIRE(dict->size() == 2);
  REQUIRE(dict->at("bar")->get_type() == BencodeType::String);
  REQUIRE(dict->at("foo")->get_type() == BencodeType::Integer);
}

TEST_CASE("[BencodeValue::BencodeDict] Dict with a list") {
  // Creating list
  std::vector<std::unique_ptr<BencodeValue>> v;
  v.push_back(std::make_unique<BencodeInt>(1));
  v.push_back(std::make_unique<BencodeString>("spam"));
  // Creating dict
  std::map<std::string, std::unique_ptr<BencodeValue>> m;
  m.insert({"bar", std::make_unique<BencodeString>("spam")});
  m.insert({"foo", std::make_unique<BencodeInt>(42)});
  m.insert({"list", std::make_unique<BencodeList>(std::move(v))});
  BencodeDict d = BencodeDict(std::move(m));
  // Check correct dict
  REQUIRE(d.to_string() == "d3:bar4:spam3:fooi42e4:listli1e4:spamee");
  REQUIRE(d.get_type() == BencodeType::Dict);
  // Check correct dict values
  auto dict = &d.value();
  REQUIRE(dict->size() == 3);
  REQUIRE(dict->at("bar")->get_type() == BencodeType::String);
  REQUIRE(dict->at("foo")->get_type() == BencodeType::Integer);
  REQUIRE(dict->at("list")->get_type() == BencodeType::List);
}
