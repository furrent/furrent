#include "asio.hpp"

using asio::ip::tcp;

namespace fur::download {
using AsioSocket = tcp::socket;
template <>
struct Socket<AsioSocket> {
  explicit Socket<AsioSocket>(AsioSocket inner) : inner{std::move(inner)} {}

  static Socket<AsioSocket> connect(const peer::Peer& peer) {
    asio::io_context io_context;

    tcp::socket _socket(io_context);
    auto _ip = asio::ip::address_v4(peer.ip);
    auto _port = asio::ip::port_type(peer.port);
    _socket.connect(tcp::endpoint(_ip, _port));

    return Socket<AsioSocket>(std::move(_socket));
  }

  void write(const std::vector<uint8_t>& buf) {
    this->inner.write_some(asio::buffer(buf));
  }

  std::vector<uint8_t> read(int n) {
    std::vector<uint8_t> buf;
    buf.resize(n);

    this->inner.read_some(asio::buffer(buf));
    return buf;
  }

  [[nodiscard]] bool is_open() const { return this->inner.is_open(); }

 private:
  AsioSocket inner;
};
}  // namespace fur::download
