#include <iostream>

#include "catch2/catch.hpp"
#include "download/connection_pool.hpp"
#include "download/handshake.hpp"
#include "download/socket.hpp"
#include "hash.hpp"

using namespace fur::download;
using namespace fur::peer;

TEST_CASE("[Download] Handshake") {
  // A public TCP echo server
  Peer peer{2130706433, 4242};
  auto pool = ConnectionPool<AsioSocket>();
  auto& sock = pool.get(peer);
  REQUIRE(sock.is_open());
  fur::hash::hash_t some_hash = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  handshake(sock, some_hash);
}
