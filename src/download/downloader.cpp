#include "download/downloader.hpp"

#include "asio.hpp"

Downloader::Downloader(Peer peer) : peer{peer} {}

void Downloader::ensure_connected() {
  if (socket.has_value() && socket->is_open()) return;

  asio::io_context io_context;

  asio::ip::tcp::socket sock(io_context);
  auto ip = asio::ip::address_v4(peer.ip);
  auto port = asio::ip::port_type(peer.port);
  sock.connect(asio::ip::tcp::endpoint(ip, port));

  socket = std::move(sock);
}
