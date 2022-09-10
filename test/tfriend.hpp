#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include "download/bitfield.hpp"
#include "download/downloader.hpp"
#include "download/socket.hpp"
#include "util/result.hpp"

using namespace fur::download::socket;
using namespace fur::download::downloader;
using namespace fur::download::bitfield;

/// This struct is befriended by classes that compose the main application in
/// order to allow private members to be accessed, for testing purposes.
struct TestingFriend {
 public:
  static Outcome<DownloaderError> Downloader_ensure_connected(
      Downloader& down) {
    return down.ensure_connected();
  }

  static Result<Downloaded, DownloaderError> Downloader_try_download(
      Downloader& down, const Task& task) {
    return down.try_download(task);
  }

  static std::optional<Socket>& Downloader_socket(Downloader& down) {
    return down.socket;
  }

  static std::vector<uint8_t>& Bitfield_storage(Bitfield& bf) {
    return bf.storage;
  }
};
