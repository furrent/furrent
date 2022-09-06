#include "download/message.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "catch2/catch.hpp"
#include "download/bitfield.hpp"
#include "torrent.hpp"

using namespace fur::torrent;
using namespace fur::download::message;

TEST_CASE("[Message] Decoding basic message") {
  // Only *really* needed to decode bitfield messages but still needs to be
  // provided to all calls to `Message::decode()`
  TorrentFile torrent;
  torrent.piece_hashes.resize(9);

  SECTION("KeepAlive") {
    auto dec = Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 0});
    REQUIRE(dec.has_value());
    dynamic_cast<KeepAliveMessage&>(**dec);
  }
  SECTION("Unchoke") {
    auto dec = Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 1, 2});
    REQUIRE(dec.has_value());
    dynamic_cast<InterestedMessage&>(**dec);
  }
  SECTION("Have") {
    auto dec = Message::decode(torrent,
                               std::vector<uint8_t>{0, 0, 0, 5, 4, 0, 0, 0, 5});
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<HaveMessage&>(**dec);
    REQUIRE(m.index == 5);
  }
  SECTION("Bitfield") {
    auto dec =
        Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 7, 5, 3, 128});
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<BitfieldMessage&>(**dec);
    // The length come from the `TorrentFile` above
    REQUIRE(m.bitfield.len == 9);
    REQUIRE(m.bitfield.get_bytes() == std::vector<uint8_t>{3, 128});
  }
  SECTION("Request") {
    auto dec = Message::decode(
        torrent, std::vector<uint8_t>{0, 0, 0, 13, 6, 0, 0, 0, 1, 0, 0, 0, 2, 0,
                                      0, 0, 3});
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<RequestMessage&>(**dec);
    REQUIRE(m.index == 1);
    REQUIRE(m.begin == 2);
    REQUIRE(m.length == 3);
  }
  SECTION("Piece") {
    auto dec = Message::decode(
        torrent, std::vector<uint8_t>{0, 0, 0, 11, 7, 0, 0, 0, 1, 0, 0, 0, 2,
                                      56, 71, 23});
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<PieceMessage&>(**dec);
    REQUIRE(m.index == 1);
    REQUIRE(m.begin == 2);
    REQUIRE(m.block == std::vector<uint8_t>{56, 71, 23});
  }
}

TEST_CASE("[Message] Encode + Decode is identity function") {
  // Only *really* needed to decode bitfield messages but still needs to be
  // provided to all calls to `Message::decode()`
  TorrentFile torrent;
  torrent.piece_hashes.resize(9);

  SECTION("KeepAlive") {
    auto dec = Message::decode(torrent,
                               std::make_unique<KeepAliveMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<KeepAliveMessage&>(**dec);
  }
  SECTION("Choke") {
    auto dec =
        Message::decode(torrent, std::make_unique<ChokeMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<ChokeMessage&>(**dec);
  }
  SECTION("Unchoke") {
    auto dec =
        Message::decode(torrent, std::make_unique<UnchokeMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<UnchokeMessage&>(**dec);
  }
  SECTION("Interested") {
    auto dec = Message::decode(torrent,
                               std::make_unique<InterestedMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<InterestedMessage&>(**dec);
  }
  SECTION("NotInterested") {
    auto dec = Message::decode(
        torrent, std::make_unique<NotInterestedMessage>()->encode());
    REQUIRE(dec.has_value());
    dynamic_cast<NotInterestedMessage&>(**dec);
  }
  SECTION("Have") {
    auto dec =
        Message::decode(torrent, std::make_unique<HaveMessage>(2167)->encode());
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<HaveMessage&>(**dec);
    REQUIRE(m.index == 2167);
  }
  SECTION("Bitfield") {
    auto dec =
        Message::decode(torrent, std::make_unique<BitfieldMessage>(
                                     Bitfield{std::vector<uint8_t>{3, 128}, 9})
                                     ->encode());
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<BitfieldMessage&>(**dec);
    REQUIRE(m.bitfield.get(6));
    REQUIRE(m.bitfield.get(7));
    REQUIRE(m.bitfield.get(8));
  }
  SECTION("Request") {
    auto dec = Message::decode(
        torrent, std::make_unique<RequestMessage>(2167, 3463, 853)->encode());
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<RequestMessage&>(**dec);
    REQUIRE(m.index == 2167);
    REQUIRE(m.begin == 3463);
    REQUIRE(m.length == 853);
  }
  SECTION("Request") {
    auto dec = Message::decode(torrent,
                               std::make_unique<PieceMessage>(
                                   2167, 3463, std::vector<uint8_t>{56, 71, 23})
                                   ->encode());
    REQUIRE(dec.has_value());
    auto m = dynamic_cast<PieceMessage&>(**dec);
    REQUIRE(m.index == 2167);
    REQUIRE(m.begin == 3463);
    REQUIRE(m.block == std::vector<uint8_t>{56, 71, 23});
  }
}

TEST_CASE("[Message] Invalid messages") {
  // Only *really* needed to decode bitfield messages but still needs to be
  // provided to all calls to `Message::decode()`
  TorrentFile torrent;
  torrent.piece_hashes.resize(9);

  SECTION("Too short") {
    REQUIRE(Message::decode(torrent, std::vector<uint8_t>{0, 0}) ==
            std::nullopt);
  }
  SECTION("No ID") {
    REQUIRE(Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 1}) ==
            std::nullopt);
  }
  SECTION("Unknown ID") {
    REQUIRE(Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 1, 99}) ==
            std::nullopt);
  }
  SECTION("HaveMessage is too short") {
    REQUIRE(Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 3, 4, 0,
                                                          0}) == std::nullopt);
  }
  SECTION("RequestMessage is too short") {
    REQUIRE(Message::decode(
                torrent, std::vector<uint8_t>{0, 0, 0, 10, 6, 0, 0, 0, 1, 0, 0,
                                              0, 2, 0}) == std::nullopt);
  }
}
