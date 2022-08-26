#include "download/message.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "catch2/catch.hpp"

using namespace fur::download::message;

TEST_CASE("[Message] Decoding basic message") {
  SECTION("KeepAlive") {
    auto dec = Message::decode(std::vector<uint8_t>{0, 0, 0, 0});
    REQUIRE(dec.has_value());
    dynamic_cast<KeepAliveMessage&>(**dec);
  }
  SECTION("Unchoke") {
    auto dec = Message::decode(std::vector<uint8_t>{0, 0, 0, 1, 2});
    REQUIRE(dec.has_value());
    dynamic_cast<InterestedMessage&>(**dec);
  }
  SECTION("Have") {
    auto dec = Message::decode(std::vector<uint8_t>{0, 0, 0, 4, 4, 0, 0, 0, 5});
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<HaveMessage&>(**dec);
    REQUIRE(m.index == 5);
  }
}

TEST_CASE("[Message] Encode + Decode is identity function") {
  SECTION("KeepAlive") {
    auto dec = Message::decode(std::make_unique<KeepAliveMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<KeepAliveMessage&>(**dec);
  }
  SECTION("Choke") {
    auto dec = Message::decode(std::make_unique<ChokeMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<ChokeMessage&>(**dec);
  }
  SECTION("Unchoke") {
    auto dec = Message::decode(std::make_unique<UnchokeMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<UnchokeMessage&>(**dec);
  }
  SECTION("Interested") {
    auto dec = Message::decode(std::make_unique<InterestedMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<InterestedMessage&>(**dec);
  }
  SECTION("NotInterested") {
    auto dec =
        Message::decode(std::make_unique<NotInterestedMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<NotInterestedMessage&>(**dec);
  }
  SECTION("Have") {
    auto dec = Message::decode(std::make_unique<HaveMessage>(2167)->encode());
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<HaveMessage&>(**dec);
    REQUIRE(m.index == 2167);
  }
}

TEST_CASE("[Message] Invalid messages") {
  SECTION("Too short") {
    REQUIRE(Message::decode(std::vector<uint8_t>{0, 0}) == std::nullopt);
  }
  SECTION("No ID") {
    REQUIRE(Message::decode(std::vector<uint8_t>{0, 0, 0, 1}) == std::nullopt);
  }
  SECTION("Unknown ID") {
    REQUIRE(Message::decode(std::vector<uint8_t>{0, 0, 0, 1, 99}) ==
            std::nullopt);
  }
  SECTION("HaveMessage is too short") {
    REQUIRE(Message::decode(std::vector<uint8_t>{0, 0, 0, 3, 4, 0, 0}) ==
            std::nullopt);
  }
}
