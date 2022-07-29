#include "peer.hpp"

#include "catch2/catch.hpp"

TEST_CASE("[Peer] Address formatting") {
  peer::Peer peer{1 << 24 | 2 << 16 | 3 << 8 | 4, 6565};
  REQUIRE(peer.address() == "1.2.3.4:6565");
}

TEST_CASE("[Peer] Peer's address formatting") {
  peer::Peer peer{1 << 24 | 2 << 16 | 3 << 8 | 4, 6565};
  REQUIRE(peer.address() == "1.2.3.4:6565");
}

// Not publicly declared in "src/peer.hpp" because not really part of the public
// API
namespace peer {
peer::AnnounceResult parse_tracker_response(const std::string& text);
}

TEST_CASE("[Peer] Parse tracker response") {
  char raw[] = "d8:intervali900e5:peers6:" "\xc0\x00\x02\x7b\x1a\xe1" "e";
  peer::AnnounceResult result = peer::parse_tracker_response(std::string{raw, sizeof(raw)-1});
  REQUIRE(result.interval == 900);
  REQUIRE(result.peers.size() == 1);

  peer::Peer peer = result.peers[0];
  REQUIRE(peer.address() == "192.0.2.123:6881");
}
