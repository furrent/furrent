#include "bencode/bencode_parser.hpp"

#include "catch2/catch.hpp"
using namespace fur::bencode;

TEST_CASE("[BencodeParser::decode()] Correct decode of a integer"){
  BencodeParser parser{};
  // Positive integer
  auto r_1 = parser.decode("i42e");
  REQUIRE(r_1);
  auto b_1 = dynamic_cast<BencodeInt&>(*(*r_1));
  REQUIRE(b_1.to_string() == "i42e");
  REQUIRE(b_1.get_type() == BencodeType::Integer);
  REQUIRE(b_1.value() == 42);
  // Negative integer
  auto r_2 = parser.decode("i-42e");
  REQUIRE(r_2);
  auto b_2 = dynamic_cast<BencodeInt&>(*(*r_2));
  REQUIRE(b_2.to_string() == "i-42e");
  REQUIRE(b_2.get_type() == BencodeType::Integer);
  REQUIRE(b_2.value() == -42);
}

TEST_CASE("[BencodeParser::decode()] Wrong decode of a integer"){
  BencodeParser parser{};
  // Missing 'e'
  auto r_1 = parser.decode("i42");
  REQUIRE(!r_1);
  REQUIRE(r_1.error() == fur::util::Error::DecodeIntFormat);
  // Negative zero is not allowed
  auto r_2 = parser.decode("i-0e");
  REQUIRE(!r_2);
  REQUIRE(r_2.error() == fur::util::Error::DecodeIntValue);
}

TEST_CASE("[BencodeParser::decode()] Correct decode of a string") {
  BencodeParser parser{};
  auto r_1 = parser.decode("4:spam");
  REQUIRE(r_1);
  auto b_1 = dynamic_cast<BencodeString&>(*(*r_1));
  REQUIRE(b_1.to_string() == "4:spam");
  REQUIRE(b_1.get_type() == BencodeType::String);
  REQUIRE(b_1.value() == "spam");
}

TEST_CASE("[BencodeParser::decode()] Wrong decode of a string") {
  BencodeParser parser{};
  // Wrong length
  auto r_1 = parser.decode("10:spam");
  REQUIRE(!r_1);
  REQUIRE(r_1.error() == fur::util::Error::DecodeStringLength);
  // Negative length
  auto r_2 = parser.decode("-4:spam");
  REQUIRE(!r_2);
  REQUIRE(r_2.error() == fur::util::Error::DecodeStringLength);
  // No colon
  //REQUIRE_THROWS_AS(parser.decode("4spam"), std::invalid_argument);
}
/*
TEST_CASE("[BencodeParser::decode()] Correct decode of a list"){
  BencodeParser parser{};
  auto list = parser.decode("l4:spami42ee");
  auto& b = dynamic_cast<BencodeList&>(*list);
  REQUIRE(b.to_string() == "l4:spami42ee");
  REQUIRE(b.get_type() == BencodeType::List);
  REQUIRE(b.value().size() == 2);
  REQUIRE(dynamic_cast<BencodeString&>(*b.value()[0]).value() == "spam");
  REQUIRE(dynamic_cast<BencodeInt&>(*b.value()[1]).value() == 42);
}

TEST_CASE("[BencodeParser::decode()] Wrong decode of a list"){
  BencodeParser parser{};
  // Missing "l" at the start
  REQUIRE_THROWS_AS(parser.decode("i42ei42ee"), std::invalid_argument);
  // Missing "e" at the end
  REQUIRE_THROWS_AS(parser.decode("li42ei42e"), std::invalid_argument);
}

TEST_CASE("[BencodeParser::decode()] Correct decode of a dictionary"){
  BencodeParser parser{};
  auto dict = parser.decode("d3:bar4:spam3:fooi42ee");
  auto& b = dynamic_cast<BencodeDict&>(*dict);
  REQUIRE(b.to_string() == "d3:bar4:spam3:fooi42ee");
  REQUIRE(b.get_type() == BencodeType::Dict);
  REQUIRE(b.value().size() == 2);
  REQUIRE(dynamic_cast<BencodeString&>(*b.value()["bar"]).value() == "spam");
  REQUIRE(dynamic_cast<BencodeInt&>(*b.value()["foo"]).value() == 42);
}

TEST_CASE("[BencodeParser::decode()] Wrong decode of a dictionary"){
  BencodeParser parser{};
  // Missing "d" at the start
  REQUIRE_THROWS_AS(parser.decode("d3:bar4:spam3:fooi42e"), std::invalid_argument);
  // Missing "e" at the end
  REQUIRE_THROWS_AS(parser.decode("d3:bar4:spam3:fooi42e"), std::invalid_argument);
  // Wrong order of the keys
  REQUIRE_THROWS_AS(parser.decode("d3:foo4:spam3:bari42ee"), std::invalid_argument);
}

TEST_CASE("[BencodeParser::encode()] Correct encode of BencodeValue"){
  // The function is the same of the to_string, so the test are already
  // implemented in BencodeValue
  BencodeParser parser{};
  // Integer
  auto b_1 = BencodeInt{42};
  REQUIRE(parser.encode(b_1) == "i42e");
  // String
  auto b_2 = BencodeString{"spam"};
  REQUIRE(parser.encode(b_2) == "4:spam");
  // List
  std::vector<std::unique_ptr<BencodeValue>> v;
  v.push_back(std::make_unique<BencodeInt>(42));
  v.push_back(std::make_unique<BencodeString>("spam"));
  auto b_4 = BencodeList{std::move(v)};
  REQUIRE(parser.encode(b_4) == "li42e4:spame");
  // Dictionary
  std::map<std::string, std::unique_ptr<BencodeValue>> m;
  m["bar"] = std::make_unique<BencodeString>("spam");
  m["foo"] = std::make_unique<BencodeInt>(42);
  auto b_5 = BencodeDict{std::move(m)};
  REQUIRE(parser.encode(b_5) == "d3:bar4:spam3:fooi42ee");

}

TEST_CASE("[BencodeParser::decode()] No invalid length of a string with 'i' chars") {
  BencodeParser parser{};
  //auto b = parser.decode("l8:intervali3ee");
  //REQUIRE(b->to_string() == "l8:intervali3ee");
  //// Check if the values are correct
  //auto& b_list = dynamic_cast<BencodeList&>(*b);
  //auto& b_string = dynamic_cast<BencodeString&>(*b_list.value()[0]);
  //REQUIRE(b_string.value() == "interval");
  //auto& b_int = dynamic_cast<BencodeInt&>(*b_list.value()[1]);
  //REQUIRE(b_int.value() == 3);

}

*/