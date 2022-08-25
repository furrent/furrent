#pragma once

#include <cstdint>
#include <vector>

#include "asio.hpp"

namespace fur::download::socket {
using timeout = std::chrono::steady_clock::duration;

/// A `Socket` wraps an `asio::ip::tcp::socket` providing a simplified interface
/// enhanced with timeout support. All methods are blocking and don't spawn
/// any more threads.
class Socket {
 public:
  /// Attempt connecting to a TCP server within the given timeout.
  void connect(uint32_t ip, uint16_t port, timeout timeout);

  /// Returns `true` if the socket is open
  bool is_open();

  /// Attempt to write all the bytes in `buf` to the socket with the given
  /// timeout.
  void write(const std::vector<uint8_t>& buf, timeout timeout);
  /// Attempt to readn `n` bytes from the socket with the given timeout.
  std::vector<uint8_t> read(int n, timeout timeout);

 private:
  /// Asynchronous runtime. Restarted before each operation.
  asio::io_context io;
  /// The wrapped socket.
  asio::ip::tcp::socket socket{io};

  /// Run any asynchronous operation scheduled on the `asio::io_context` with
  /// the specified timeout.
  void run(timeout timeout);
};
}  // namespace fur::download::socket
