#pragma once

#include "download/downloader.hpp"

/// This struct is befriended by classes that compose the main application in
/// order to allow private members to be accessed, for testing purposes.
struct TestingFriend {
 public:
  static void Downloader_ensure_connected(
      fur::download::downloader::Downloader& down) {
    down.ensure_connected();
  }
};
