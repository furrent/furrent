#include "download/downloader.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <thread>
#include <vector>

#include "bencode/bencode_parser.hpp"
#include "catch2/catch.hpp"
#include "torrent.hpp"
#include "hash.hpp"
#include "log/logger.hpp"
#include "peer.hpp"
#include "tfriend.hpp"

using namespace fur;
using namespace fur::peer;
using namespace fur::bencode;
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
  REQUIRE(TestingFriend::Downloader_ensure_connected(down).valid());
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

  std::vector<Subpiece> subpieces = { Subpiece{ "Subpiece", 0, torrent.piece_length } };
  auto maybe_downloaded = TestingFriend::Downloader_try_download(down, Piece{
    0u, subpieces});

  REQUIRE(maybe_downloaded.valid());
  auto downloaded = *maybe_downloaded;

  // 16384 is 16KB
  REQUIRE(downloaded.content.size() == 16384);
}

void test_alice(std::vector<DownloaderError>& errors) {
  // Faker on port 4006 seeds a whole alice.txt file contained in the fixtures/
  // directory

  Peer peer("127.0.0.1", 4006);

  // Read torrent file to string
  std::ifstream f("../test/fixtures/alice.txt.torrent");
  std::stringstream buf;
  buf << f.rdbuf();
  std::string content = buf.str();

  // Parse bencode
  BencodeParser parser;
  auto ben_tree = parser.decode(content);

  // Parse TorrentFile
  TorrentFile torrent(*(*ben_tree));

  Downloader down(torrent, peer);

  std::vector<int> pieces_left{0, 1, 2, 3, 4};
  auto original_pieces_left = pieces_left;

  while (!pieces_left.empty()) {
    // Must not mutate original array while iterating
    auto pieces_left_copy = pieces_left;
    for (auto idx : pieces_left_copy) {

      std::vector<Subpiece> subpieces = { Subpiece{ 0, 0, torrent.piece_length } };
      auto maybe_downloaded =
          TestingFriend::Downloader_try_download(down, Piece{
            static_cast<size_t>(idx), subpieces});
      if (!maybe_downloaded.valid()) {
        spdlog::get("custom")->error(
            "error in downloader: {}",
            display_downloader_error(maybe_downloaded.error()));

        auto erase_me =
            std::find(errors.begin(), errors.end(), maybe_downloaded.error());
        if (erase_me != errors.end()) {
          errors.erase(erase_me);
        }
        continue;
      }

      pieces_left.erase(std::find(pieces_left.begin(), pieces_left.end(), idx));
    }

    // The faker has none of the remaining pieces. The faker only
    // simulates acquiring a new piece when receiving requests from us, but
    // we're not sending any, so we're stuck a loop because `try_download` keeps
    // returning `std::nullopt`. Kickstart the whole process again.
    if (pieces_left_copy.size() == pieces_left.size()) {
      pieces_left = original_pieces_left;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

// This is quite a time-consuming test, feel free to skip when running other
// tests locally
TEST_CASE("[Downloader] Download alice") {
  // Assert that all possible errors are encountered at least once
  std::vector<DownloaderError> errors{
      DownloaderError::DifferentInfoHash, DownloaderError::InvalidMessage,
      DownloaderError::NoBitfield,        DownloaderError::MissingPiece,
      DownloaderError::CorruptPiece,      DownloaderError::SocketTimeout,
      DownloaderError::SocketOther,
  };

  // Do it a couple of times because the alice faker is non-deterministic and
  // may not always stress all `Downloader` behavior patterns
  for (int i = 0; i < 30; i++) {
    test_alice(errors);
  }

  REQUIRE(errors.empty());
}