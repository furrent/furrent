#include "bencode_parser.hpp"

#include "catch.hpp"
using namespace fur::bencode;

TEST_CASE("[BencodeParser::decode()] Correct decode of a integer"){
  BencodeParser parser{};
  // Positive integer
  auto b_1 = dynamic_cast<BencodeInt&>(*parser.decode("i42e"));
  REQUIRE(b_1.to_string() == "i42e");
  REQUIRE(b_1.get_type() == BencodeType::Integer);
  REQUIRE(b_1.value() == 42);
  // Negative integer
  auto b_2 = dynamic_cast<BencodeInt&>(*parser.decode("i-42e"));
  REQUIRE(b_2.to_string() == "i-42e");
  REQUIRE(b_2.get_type() == BencodeType::Integer);
  REQUIRE(b_2.value() == -42);
}

TEST_CASE("[BencodeParser::decode()] Wrong decode of a integer"){
  BencodeParser parser{};
  REQUIRE_THROWS_AS(parser.decode("42"), std::invalid_argument);
  REQUIRE_THROWS_AS(parser.decode("i-0e"), std::invalid_argument);
  REQUIRE_THROWS_AS(parser.decode("i42"), std::invalid_argument);
  REQUIRE_THROWS_AS(parser.decode("42e"), std::invalid_argument);
}

TEST_CASE("[BencodeParser::decode()] Correct decode of a string") {
  BencodeParser parser{};
  auto b_1 = dynamic_cast<BencodeString&>(*parser.decode("4:spam"));
  REQUIRE(b_1.to_string() == "4:spam");
  REQUIRE(b_1.get_type() == BencodeType::String);
  REQUIRE(b_1.value() == "spam");
}

TEST_CASE("[BencodeParser::decode()] Wrong decode of a string") {
  BencodeParser parser{};
  // Wrong length
  REQUIRE_THROWS_AS(parser.decode("3:spam"), std::invalid_argument);
  // Negative length
  REQUIRE_THROWS_AS(parser.decode("-4:spam"), std::invalid_argument);
  // No colon
  REQUIRE_THROWS_AS(parser.decode("4spam"), std::invalid_argument);
}

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
  auto b = parser.decode("l8:intervali3ee");
  REQUIRE(b->to_string() == "l8:intervali3ee");
  // Check if the values are correct
  auto& b_list = dynamic_cast<BencodeList&>(*b);
  auto& b_string = dynamic_cast<BencodeString&>(*b_list.value()[0]);
  REQUIRE(b_string.value() == "interval");
  auto& b_int = dynamic_cast<BencodeInt&>(*b_list.value()[1]);
  REQUIRE(b_int.value() == 3);

}


