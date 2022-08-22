#include <cstdint>
#include <stdexcept>

#include "download/socket.hpp"
#include "hash.hpp"

namespace fur::download {
// 1  for the length of the protocol identifier
// 19 for the protocol identifier itself
// 8  for the extensions bits
// 20 for the info hash
// 20 for the peer id
const int HANDSHAKE_LENGTH = 1 + 19 + 8 + 20 + 20;
const int INFO_HASH_OFFSET = 1 + 19 + 8;

template <typename T>
void handshake(Socket<T>& socket, hash::hash_t info_hash) {
  static_assert(Socket<T>::valid, "T is not a Socket");

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
  message.insert(message.end(), info_hash.begin(), info_hash.end());

  auto peerId = std::string("FUR-----------------");
  // Peer id
  message.insert(message.end(), peerId.begin(), peerId.end());

  socket.write(message);

  auto response = socket.read(HANDSHAKE_LENGTH);
  hash::hash_t response_info_hash;
  std::copy(response.begin() + INFO_HASH_OFFSET,
            response.begin() + INFO_HASH_OFFSET + sizeof(hash::hash_t{}),
            response_info_hash.begin());

  if (info_hash != response_info_hash) {
    throw std::runtime_error("bad handshake");
  }
}
}  // namespace fur::download
