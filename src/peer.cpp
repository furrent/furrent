#include "peer.hpp"

#include "fmt/core.h"

namespace peer {
std::string Peer::address() const {
  return fmt::format("{}.{}.{}.{}:{}", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                     (ip >> 8) & 0xFF, ip & 0xFF, port);
}
}  // namespace peer
