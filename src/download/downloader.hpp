#pragma once

#include <optional>

#include "asio.hpp"
#include "peer.hpp"

using namespace fur::peer;

using Socket = asio::ip::tcp::socket;

class Downloader {
 private:
  const Peer peer;
  std::optional<Socket> socket;

 public:
  explicit Downloader(Peer peer);
  void ensure_connected();
};
