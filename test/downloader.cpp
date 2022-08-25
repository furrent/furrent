#include "download/downloader.hpp"

#include "catch2/catch.hpp"
#include "hash.hpp"
#include "peer.hpp"
#include "tfriend.hpp"

using namespace fur::peer;
using namespace fur::download::downloader;

TEST_CASE("[Download] Handshake") {
  // Localhost faker
  Peer peer("127.0.0.1", 4242);
  TorrentFile torrent{};
  torrent.info_hash = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                       11, 12, 13, 14, 15, 16, 17, 18, 19, 20};

  Downloader down(torrent, peer);

  TestingFriend::Downloader_ensure_connected(down);
}
