#include "download/socket.hpp"

#include <system_error>

#include "log/logger.hpp"

namespace fur::download::socket {
Outcome<SocketError> Socket::connect(uint32_t ip, uint16_t port,
                                     timeout timeout) {
  asio::ip::tcp::endpoint endpoint{asio::ip::address_v4{ip},
                                   asio::ip::port_type{port}};
  // For whatever reason, `asio::async_connect` doesn't accept a single
  // endpoint. You need to pass an endpoint iterator and `std::array` is
  // probably the simplest iterator.
  std::array<asio::ip::tcp::endpoint, 1> endpoints{endpoint};

  // Error code set by the socket's callback.
  std::error_code ec;
  // Schedule an asynchronous operation that will attempt to connect the socket
  // to the provided endpoint. When the operation yields (either because it
  // succeeded or because the timeout expired) the callback exports the error
  // code to the variable above.
  asio::async_connect(socket, endpoints,
                      [&](const std::error_code& in_ec,
                          const asio::ip::tcp::endpoint&) { ec = in_ec; });

  run(timeout);

  // If the test passes, then either the timeout expired before the socket
  // could connect or there was some other unexpected error.
  if (ec) {
    auto logger = spdlog::get("custom");
    logger->error("connecting socket: {}", ec.message());

    switch (ec.value()) {
      case asio::error::operation_aborted:
        return Outcome<SocketError>::ERROR(SocketError::Timeout);
      default:
        return Outcome<SocketError>::ERROR(SocketError::Other);
    }
  }

  return Outcome<SocketError>::OK({});
}

bool Socket::is_open() { return socket.is_open(); }

Outcome<SocketError> Socket::write(const std::vector<uint8_t>& buf,
                                   timeout timeout) {
  // Error code set by the socket's callback.
  std::error_code ec;
  // Schedule an asynchronous operation that will attempt to write the given
  // buffer to the socket. When the operation yields (either because it
  // succeeded or because the timeout expired) the callback exports the error
  // code to the variable above.
  //
  // Note that the `std::size_t` in the closure's arguments that reports the
  // number of bytes written might not match the size of `buf`. That's because
  // an asynchronous function can yield before completion and resume later.
  // If the asynchronous runtime is able to complete the operation within the
  // timeout bounds, we can stand assured that all bytes have been written.
  asio::async_write(
      socket, asio::buffer(buf),
      [&](const std::error_code& in_ec, std::size_t) { ec = in_ec; });

  run(timeout);

  // If the test passes, then either the timeout expired before the socket
  // could write all the bytes or there was some other unexpected error.
  if (ec) {
    auto logger = spdlog::get("custom");
    logger->error("writing to socket: {}", ec.message());

    switch (ec.value()) {
      case asio::error::operation_aborted:
        return Outcome<SocketError>::ERROR(SocketError::Timeout);
      default:
        return Outcome<SocketError>::ERROR(SocketError::Other);
    }
  }

  return Outcome<SocketError>::OK({});
}

Result<std::vector<uint8_t>, SocketError> Socket::read(uint32_t n,
                                                       timeout timeout) {
  using Result = Result<std::vector<uint8_t>, SocketError>;

  std::vector<uint8_t> buf{};
  buf.resize(n);

  // Error code set by the socket's callback.
  std::error_code ec;

  // Schedule an asynchronous operation that will attempt to read `n` bytes
  // from the socket. When the operation yields (either because it succeeded or
  // because the timeout expired) the callback exports the error code to the
  // variable above.
  //
  // Note that the `std::size_t` in the closure's arguments that reports the
  // number of bytes read might not match `n`. That's because an asynchronous
  // function can yield before completion and resume later. If the asynchronous
  // runtime is able to complete the operation within the timeout bounds, we can
  // stand assured that all bytes have been read.
  asio::async_read(
      socket, asio::buffer(buf),
      [&](const std::error_code& in_ec, std::size_t) { ec = in_ec; });

  run(timeout);

  // If the test passes, then either the timeout expired before the socket
  // could read `n` bytes or there was some other unexpected error.
  if (ec) {
    auto logger = spdlog::get("custom");
    logger->error("reading from socket: {}", ec.message());

    switch (ec.value()) {
      case asio::error::operation_aborted:
        return Result::ERROR(SocketError::Timeout);
      default:
        return Result::ERROR(SocketError::Other);
    }
  }

  return Result::OK(std::move(buf));
}

void Socket::run(timeout timeout) {
  // Restart the asynchronous runtime that might have been left in a
  // "stopped" state by a previous operation.
  io.restart();

  // Run all scheduled asynchronous operations until completion unless the
  // timeout expires.
  io.run_for(timeout);

  // A stopped runtime indicates that all scheduled operations terminated
  // successfully. If that's not the case, then the timeout expired.
  if (!io.stopped()) {
    try {
      // Ask the socket to cancel any pending task.
      socket.shutdown(asio::ip::tcp::socket::shutdown_both);
    } catch (const asio::system_error& _) {
      // We're calling `shutdown` because we're benevolent gods, but it doesn't
      // really matter if it fails.
    }

    try {
      // Close the underlying socket.
      socket.close();
    } catch (const asio::system_error& err) {
      auto logger = spdlog::get("custom");
      logger->error("closing socket after timeout elapsed: {}", err.what());

      // Don't bubble up the error because this is non-critical. ASIO guarantees
      // that the underlying socket is closed anyway.
    }

    // Run all to completion to allow the socket to clean up.
    io.run();
  }
}

Outcome<SocketError> Socket::close() {
  try {
    // Ask the socket to cancel any pending task.
    socket.shutdown(asio::ip::tcp::socket::shutdown_both);
  } catch (const asio::system_error& _) {
    // We're calling `shutdown` because we're benevolent gods, but it doesn't
    // really matter if it fails.
  }

  try {
    // Close the underlying socket.
    socket.close();
  } catch (const asio::system_error& err) {
    auto logger = spdlog::get("custom");
    logger->error("closing socket: {}", err.what());

    return Outcome<SocketError>::ERROR(SocketError::Other);
  }

  // Run all to completion to allow the socket to clean up.
  io.run();

  return Outcome<SocketError>::OK({});
}
}  // namespace fur::download::socket
