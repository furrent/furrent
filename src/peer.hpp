#pragma once

#include <cstdint>
#include <string>

#include "fmt/core.h"

/**
 * Contains data structures and facilities for representing and discovering
 * BitTorrent peers
 */
namespace peer {
/**
 * Represents a single peer as given by the tracker
 */
struct Peer {
  uint32_t ip;
  uint16_t port;

  /**
   * Combines the ip and port of the peer into a X.Y.Z.W:PORT string
   * @return The combined string
   */
  [[nodiscard]] std::string address() const;
};

std::string Peer::address() const {
  return fmt::format("{}.{}.{}.{}:{}", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                     (ip >> 8) & 0xFF, ip & 0xFF, port);
}
}  // namespace peer
