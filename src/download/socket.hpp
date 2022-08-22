#pragma once

#include <cstdint>

#include "peer.hpp"

namespace fur::download {
/// A Socket is any type capable of acting like one. Currently, only asio is
/// supported.
template <typename T>
struct Socket {
  static const bool valid = false;

  static Socket<T> connect(const peer::Peer&);

  void write(const std::vector<uint8_t>&);
  std::vector<uint8_t> read(int n);

  [[nodiscard]] bool is_open() const;
};
}  // namespace fur::download

#include "socket_asio.hpp"

// TODO
//  - Remove connection pool which is useless
//  - Add a Peer pool
//  - Make a Downloader struct that embeds a Peer and a socket
