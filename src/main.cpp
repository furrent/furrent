#include <cstdint>
#include <fstream>
#include <iostream>
#include <log/logger.hpp>
#include <sstream>

#include "bencode/bencode_parser.hpp"
#include "download/downloader.hpp"
#include "peer.hpp"
#include "torrent.hpp"

const char* TORRENT_1 = "../extra/debian-11.4.0-amd64-netinst.iso.torrent";
const char* TORRENT_2 = "../extra/ubuntu-22.04.1-desktop-amd64.iso.torrent";

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
