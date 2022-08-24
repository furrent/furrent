#include "download/downloader.hpp"

#include <array>

#include "asio.hpp"

namespace fur::downloader {
Downloader::Downloader(const TorrentFile& torrent, const Peer& peer)
    : torrent{torrent}, peer{peer} {}

void Downloader::ensure_connected() {
  if (socket.has_value() && socket->is_open()) return;

  asio::io_context io_context;

  asio::ip::tcp::socket sock(io_context);
  auto ip = asio::ip::address_v4(peer.ip);
  auto port = asio::ip::port_type(peer.port);
  sock.connect(asio::ip::tcp::endpoint(ip, port));

  socket = std::move(sock);

  handshake();
}

// 1  for the length of the protocol identifier
// 19 for the protocol identifier itself
// 8  for the extensions bits
// 20 for the info hash
// 20 for the peer id
const int HANDSHAKE_LENGTH = 1 + 19 + 8 + 20 + 20;
const int INFO_HASH_OFFSET = 1 + 19 + 8;

void Downloader::handshake() {
  std::vector<uint8_t> message;
  message.reserve(HANDSHAKE_LENGTH);

  // Length of protocol identifier
  message.push_back(19);

  auto protocol = std::string("BitTorrent protocol");
  // Protocol identifier
  message.insert(message.end(), protocol.begin(), protocol.end());

  // 8 zeroes to indicate that we support no extensions
  message.resize(message.size() + 8);

  // Info hash
  message.insert(message.end(), torrent.info_hash.begin(),
                 torrent.info_hash.end());

  auto peerId = std::string("FUR-----------------");
  // Peer id
  message.insert(message.end(), peerId.begin(), peerId.end());

  socket->write_some(asio::buffer(message));

  std::array<uint8_t, HANDSHAKE_LENGTH> response{};
  socket->read_some(asio::buffer(response));

  hash::hash_t response_info_hash;
  std::copy(response.begin() + INFO_HASH_OFFSET,
            response.begin() + INFO_HASH_OFFSET + sizeof(hash::hash_t{}),
            response_info_hash.begin());

  if (torrent.info_hash != response_info_hash) {
    throw std::runtime_error("bad handshake");
  }
}
}  // namespace fur::downloader