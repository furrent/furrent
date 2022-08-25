#pragma once

#include <optional>

#include "download/downloader.hpp"
#include "download/socket.hpp"

using namespace fur::download::socket;
using namespace fur::download::downloader;

/// This struct is befriended by classes that compose the main application in
/// order to allow private members to be accessed, for testing purposes.
struct TestingFriend {
 public:
  static void Downloader_ensure_connected(Downloader& down) {
    down.ensure_connected();
  }

  static std::optional<Socket>& Downloader_socket(Downloader& down) {
    return down.socket;
  }
};
