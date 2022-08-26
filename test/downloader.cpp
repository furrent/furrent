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
  torrent.info_hash = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  torrent.piece_hashes =
      // The faker will send us a bitfield for 3 pieces
      std::vector<hash_t>{
          // Hash for piece 1
          {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
          // Hash for piece 2
          {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
          // Hash for piece 3
          {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
      };

  Downloader down(torrent, peer);

  // Assert that the socket is not yet present (lazily initialized)
  REQUIRE(!TestingFriend::Downloader_socket(down).has_value());
  TestingFriend::Downloader_ensure_connected(down);
  // Assert that the socket is now present
  REQUIRE(TestingFriend::Downloader_socket(down).has_value());
}

TEST_CASE("[Downloader] Download piece") {
  // Faker on port 4005 does the same as the faker on 4004, except it also reads
  // a `RequestMessage` and then sends 16KB from a piece.

  Peer peer("127.0.0.1", 4005);
  TorrentFile torrent{};
  torrent.info_hash = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  torrent.piece_hashes =
      // The faker will send us a bitfield for 3 pieces
      std::vector<hash_t>{
          // Hash for piece 1
          {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
          // Hash for piece 2
          {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
          // Hash for piece 3
          {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
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
