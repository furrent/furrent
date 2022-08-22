#pragma once

#include <unordered_map>

#include "download/socket.hpp"
#include "peer.hpp"

namespace fur::download {
template <typename T>
class ConnectionPool {
  static_assert(Socket<T>::valid, "T is not a Socket");

 private:
  std::unordered_map<peer::Peer, Socket<T>> inner;

 public:
  Socket<T>& get(const peer::Peer& for_peer) {
    if (this->inner.find(for_peer) != this->inner.end()) {
      auto& socket = this->inner.at(for_peer);

      // Is it even possible for one of our sockets to be closed? No idea,
      // better safe than sorry!
      if (!socket.is_open()) {
        this->inner.erase(for_peer);
        return get(for_peer);
      }

      return socket;
    }

    // Create a new socket
    auto socket = Socket<T>::connect(for_peer);
    this->inner.insert({for_peer, std::move(socket)});

    return this->inner.at(for_peer);
  }
};
}  // namespace fur::download
