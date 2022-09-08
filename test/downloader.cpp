#include "download/downloader.hpp"

#include <vector>

#include "catch2/catch.hpp"
#include "hash.hpp"
#include "peer.hpp"
#include "tfriend.hpp"

using namespace fur::peer;
using namespace fur::download::downloader;
using namespace fur::hash;

TEST_CASE("[Downloader] Ensure connected") {
  // Faker on port 4004 will read a BitTorrent handshake message and reply with
  // a correct response (same info-hash, possibly different peer ID) then send
  // a bitfield and then unchoke us. Basically all that the `Downloader` asks
  // for to set up a new connection.

  Peer peer("127.0.0.1", 4004);
  TorrentFile torrent{};
  // The exact value doesn't really matter, it's just to verify that the peer
  // sends back an info hash that matches ours
  torrent.info_hash = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  // `piece_hashes` is only used by `ensure_connected` to check the length of
  // the bitfield, so we can just use a single zeroed hash because the faker on
  // port 4004 will send a bitfield for 1 piece.
  torrent.piece_hashes.resize(1);

  Downloader down(torrent, peer);

  // Assert that the socket is not yet present (lazily initialized)
  REQUIRE(!TestingFriend::Downloader_socket(down).has_value());
  TestingFriend::Downloader_ensure_connected(down);
  // Assert that the socket is now present
  REQUIRE(TestingFriend::Downloader_socket(down).has_value());
}

TEST_CASE("[Downloader] Download one piece, same size as a block") {
  // Faker on port 4005 does the same as the faker on 4004, except it also reads
  // a `RequestMessage` and then sends 16KB which is a whole piece.

  Peer peer("127.0.0.1", 4005);
  TorrentFile torrent{};
  torrent.length = 16384;
  torrent.piece_length = 16384;
  // The exact value doesn't really matter, it's just to verify that the peer
  // sends back an info hash that matches ours
  torrent.info_hash = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  torrent.piece_hashes = std::vector<hash_t>{
      // The faker on port 4005 will send us a 16KB block of bytes all equal to
      // 1. You can check that this is its SHA1 hash.
      {25,  229, 220, 52,  237, 167, 156, 163, 238, 115,
       134, 11,  97,  182, 97,  133, 20,  155, 111, 180},
  };

  Downloader down(torrent, peer);

  // Assert that the socket is not yet present (lazily initialized)
  REQUIRE(!TestingFriend::Downloader_socket(down).has_value());
  TestingFriend::Downloader_ensure_connected(down);
  // Assert that the socket is now present
  REQUIRE(TestingFriend::Downloader_socket(down).has_value());

  auto result = TestingFriend::Downloader_try_download(down, Task{0});
  REQUIRE(result.has_value());
  // 16384 is 16KB
  REQUIRE(result->content.size() == 16384);
}
