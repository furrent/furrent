#pragma once

#include <optional>

#include "asio.hpp"
#include "peer.hpp"
#include "tfriend_fw.hpp"
#include "torrent.hpp"

using namespace fur::peer;
using namespace fur::torrent;

namespace fur::downloader {
using Socket = asio::ip::tcp::socket;

class Downloader {
 public:
  explicit Downloader(const TorrentFile& torrent, const Peer& peer);

 private:
  const TorrentFile& torrent;
  const Peer& peer;

  std::optional<Socket> socket;

  void ensure_connected();
  void handshake();

  friend TestingFriend;
};
}  // namespace fur::downloader
