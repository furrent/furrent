#include "catch2/catch.hpp"

#include "peer.hpp"

TEST_CASE("[Peer] Address formatting") {
  peer::Peer peer {1 << 24 | 2 << 16 | 3 << 8 | 4, 6565};
  REQUIRE(peer.address() == "1.2.3.4:6565");
}

TEST_CASE("[Peer] Peer's address formatting") {
  peer::Peer peer {1 << 24 | 2 << 16 | 3 << 8 | 4, 6565};
  REQUIRE(peer.address() == "1.2.3.4:6565");
}
