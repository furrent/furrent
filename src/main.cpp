#include <cstdint>
#include <fstream>
#include <iostream>
#include <log/logger.hpp>
#include <sstream>

#include "bencode/bencode_parser.hpp"
#include "download/downloader.hpp"
#include "peer.hpp"
#include "torrent.hpp"

using namespace fur::bencode;
using namespace fur::download;
using namespace fur::download::downloader;

int main(int argc, char* argv[]) {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");

  if (argc != 4) {
    logger->error("usage: furrent <path> <ip> <port>");
    return 1;
  }

  std::string ip = std::string(argv[2]);
  uint16_t port = std::atoi(argv[3]);

  Peer peer(ip, port);

  // Read torrent file to string
  std::ifstream f(argv[1]);
  std::stringstream buf;
  buf << f.rdbuf();
  std::string content = buf.str();

  // Parse bencode
  BencodeParser parser;
  auto ben_tree = parser.decode(content);

  // Parse TorrentFile
  TorrentFile torrent(*ben_tree);

  Downloader down(torrent, peer);

  for (int i = 0; i < static_cast<int>(torrent.piece_hashes.size()); i++) {
    while (true) {
      auto maybe_downloaded = down.try_download(Task{i});
      if (!maybe_downloaded.valid()) continue;

      for (auto c : maybe_downloaded->content) {
        std::cout << c;
      }
      break;
    }
  }

  return 0;
}
