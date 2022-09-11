#pragma once

#include <cstdint>
#include <vector>

#include "asio.hpp"
#include "util/result.hpp"

using namespace fur::util;

namespace fur::download::socket {
using timeout = std::chrono::steady_clock::duration;

enum class SocketError {
  /// Socket timed out
  Timeout,
  /// Socket experienced some other generic error
  Other,
};

/// A `Socket` wraps an `asio::ip::tcp::socket` providing a simplified interface
/// enhanced with timeout support. All methods are blocking and don't spawn
/// any more threads.
class Socket {
 public:
  /// Attempt connecting to a TCP server within the given timeout.
  Outcome<SocketError> connect(uint32_t ip, uint16_t port, timeout timeout);

  /// Returns `true` if the socket is open
  bool is_open();

  /// Attempt to write all the bytes in `buf` to the socket with the given
  /// timeout.
  Outcome<SocketError> write(const std::vector<uint8_t>& buf, timeout timeout);
  /// Attempt to readn `n` bytes from the socket with the given timeout.
  Result<std::vector<uint8_t>, SocketError> read(uint32_t n, timeout timeout);

  /// Close the socket
  Outcome<SocketError> close();

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
