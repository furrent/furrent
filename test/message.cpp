#include "download/message.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "catch2/catch.hpp"
#include "download/bitfield.hpp"
#include "torrent.hpp"

using namespace fur;
using namespace fur::download::message;

TEST_CASE("[Message] Decoding basic message") {
  // Only *really* needed to decode bitfield messages but still needs to be
  // provided to all calls to `Message::decode()`
  TorrentFile torrent;
  torrent.piece_hashes.resize(9);

  SECTION("KeepAlive") {
    auto maybe_dec = Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 0});
    REQUIRE(maybe_dec.valid());
    dynamic_cast<KeepAliveMessage&>(**maybe_dec);
  }
  SECTION("Unchoke") {
    auto maybe_dec =
        Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 1, 2});
    REQUIRE(maybe_dec.valid());
    dynamic_cast<InterestedMessage&>(**maybe_dec);
  }
  SECTION("Have") {
    auto maybe_dec = Message::decode(
        torrent, std::vector<uint8_t>{0, 0, 0, 5, 4, 0, 0, 0, 5});
    REQUIRE(maybe_dec.valid());
    auto m = dynamic_cast<HaveMessage&>(**maybe_dec);
    REQUIRE(m.index == 5);
  }
  SECTION("Bitfield") {
    auto maybe_dec =
        Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 7, 5, 3, 128});
    REQUIRE(maybe_dec.valid());
    auto m = dynamic_cast<BitfieldMessage&>(**maybe_dec);
    // The length come from the `TorrentFile` above
    REQUIRE(m.bitfield.len == 9);
    REQUIRE(m.bitfield.get_bytes() == std::vector<uint8_t>{3, 128});
  }
  SECTION("Request") {
    auto maybe_dec = Message::decode(
        torrent, std::vector<uint8_t>{0, 0, 0, 13, 6, 0, 0, 0, 1, 0, 0, 0, 2, 0,
                                      0, 0, 3});
    REQUIRE(maybe_dec.valid());
    auto m = dynamic_cast<RequestMessage&>(**maybe_dec);
    REQUIRE(m.index == 1);
    REQUIRE(m.begin == 2);
    REQUIRE(m.length == 3);
  }
  SECTION("Piece") {
    auto maybe_dec = Message::decode(
        torrent, std::vector<uint8_t>{0, 0, 0, 11, 7, 0, 0, 0, 1, 0, 0, 0, 2,
                                      56, 71, 23});
    REQUIRE(maybe_dec.valid());
    auto m = dynamic_cast<PieceMessage&>(**maybe_dec);
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
    auto maybe_dec = Message::decode(
        torrent, std::make_unique<KeepAliveMessage>()->encode());
    REQUIRE(maybe_dec.valid());
    dynamic_cast<KeepAliveMessage&>(**maybe_dec);
  }
  SECTION("Choke") {
    auto maybe_dec =
        Message::decode(torrent, std::make_unique<ChokeMessage>()->encode());
    REQUIRE(maybe_dec.valid());
    dynamic_cast<ChokeMessage&>(**maybe_dec);
  }
  SECTION("Unchoke") {
    auto maybe_dec =
        Message::decode(torrent, std::make_unique<UnchokeMessage>()->encode());
    REQUIRE(maybe_dec.valid());
    dynamic_cast<UnchokeMessage&>(**maybe_dec);
  }
  SECTION("Interested") {
    auto maybe_dec = Message::decode(
        torrent, std::make_unique<InterestedMessage>()->encode());
    REQUIRE(maybe_dec.valid());
    dynamic_cast<InterestedMessage&>(**maybe_dec);
  }
  SECTION("NotInterested") {
    auto maybe_dec = Message::decode(
        torrent, std::make_unique<NotInterestedMessage>()->encode());
    REQUIRE(maybe_dec.valid());
    dynamic_cast<NotInterestedMessage&>(**maybe_dec);
  }
  SECTION("Have") {
    auto maybe_dec =
        Message::decode(torrent, std::make_unique<HaveMessage>(2167)->encode());
    REQUIRE(maybe_dec.valid());
    auto m = dynamic_cast<HaveMessage&>(**maybe_dec);
    REQUIRE(m.index == 2167);
  }
  SECTION("Bitfield") {
    auto maybe_dec =
        Message::decode(torrent, std::make_unique<BitfieldMessage>(
                                     Bitfield{std::vector<uint8_t>{3, 128}, 9})
                                     ->encode());
    REQUIRE(maybe_dec.valid());
    auto m = dynamic_cast<BitfieldMessage&>(**maybe_dec);
    REQUIRE(m.bitfield.get(6));
    REQUIRE(m.bitfield.get(7));
    REQUIRE(m.bitfield.get(8));
  }
  SECTION("Request") {
    auto maybe_dec = Message::decode(
        torrent, std::make_unique<RequestMessage>(2167, 3463, 853)->encode());
    REQUIRE(maybe_dec.valid());
    auto m = dynamic_cast<RequestMessage&>(**maybe_dec);
    REQUIRE(m.index == 2167);
    REQUIRE(m.begin == 3463);
    REQUIRE(m.length == 853);
  }
  SECTION("Request") {
    auto maybe_dec = Message::decode(
        torrent, std::make_unique<PieceMessage>(
                     2167, 3463, std::vector<uint8_t>{56, 71, 23})
                     ->encode());
    REQUIRE(maybe_dec.valid());
    auto m = dynamic_cast<PieceMessage&>(**maybe_dec);
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
    REQUIRE(!Message::decode(torrent, std::vector<uint8_t>{0, 0}).valid());
  }
  SECTION("No ID") {
    REQUIRE(
        !Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 1}).valid());
  }
  SECTION("Unknown ID") {
    REQUIRE(!Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 1, 99})
                 .valid());
  }
  SECTION("HaveMessage is too short") {
    REQUIRE(!Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 3, 4, 0, 0})
                 .valid());
  }
  SECTION("RequestMessage is too short") {
    REQUIRE(!Message::decode(torrent, std::vector<uint8_t>{0, 0, 0, 10, 6, 0, 0,
                                                           0, 1, 0, 0, 0, 2, 0})
                 .valid());
  }
}
