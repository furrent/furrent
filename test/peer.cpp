#include "peer.hpp"

#include "catch2/catch.hpp"
#include "log/logger.hpp"

using namespace fur::peer;

TEST_CASE("[Peer] Build raw") {
  REQUIRE(Peer(2130706433, 4242).address() == "127.0.0.1:4242");
  REQUIRE(Peer(0, 0).address() == "0.0.0.0:0");
  REQUIRE(Peer(4294967295, 65535).address() == "255.255.255.255:65535");
}

TEST_CASE("[Peer] Build from IP string") {
  REQUIRE(Peer("1.2.3.4", 1234).address() == "1.2.3.4:1234");
  REQUIRE(Peer("127.0.0.1", 4242).address() == "127.0.0.1:4242");
  REQUIRE(Peer("0.0.0.0", 0).address() == "0.0.0.0:0");
  REQUIRE(Peer("255.255.255.255", 65535).address() == "255.255.255.255:65535");
}

TEST_CASE("[Peer] Build from invalid IP string") {
  // When the exception is thrown is needed the logger must be initialized
  fur::log::initialize_custom_logger();
  // Disable the logger, not needed for the test
  auto logger = spdlog::get("custom");
  logger->set_level(spdlog::level::off);


  REQUIRE_THROWS(Peer("1.2.3", 4242));
  REQUIRE_THROWS(Peer("-1.2.3.4", 4242));
  REQUIRE_THROWS(Peer("1.X.3.4", 4242));
}

// Not publicly declared in "src/peer.hpp" because not really part of the public
// API
namespace fur::peer {
AnnounceResult parse_tracker_response(const std::string& text);
}

TEST_CASE("[Peer] Parse tracker response") {
  char raw[] =
      "d8:intervali900e5:peers6:"
      "\xc0\x00\x02\x7b\x1a\xe1"
      "e";
  AnnounceResult result =
      parse_tracker_response(std::string{raw, sizeof(raw) - 1});
  REQUIRE(result.interval == 900);
  REQUIRE(result.peers.size() == 1);

  Peer peer = result.peers[0];
  REQUIRE(peer.address() == "192.0.2.123:6881");
}
