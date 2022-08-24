#include "download/downloader.hpp"

#include "catch2/catch.hpp"
#include "peer.hpp"

using namespace fur::peer;

TEST_CASE("[Download] Handshake") {
  // Localhost faker
  Peer peer("127.0.0.1", 4242);
  Downloader down(peer);
}
