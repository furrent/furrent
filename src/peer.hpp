#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "torrent.hpp"

/// Contains data structures and facilities for representing and discovering
/// BitTorrent peers
namespace peer {
/// Represents a single peer as given by the tracker
struct Peer {
  uint32_t ip;
  uint16_t port;

  /// Combines the ip and port of the peer into a X.Y.Z.W:PORT string
  /// \return The combined string
  [[nodiscard]] std::string address() const;
};

/// Announce ourselves to the tracker and get a list of peers to download the
/// file from
[[nodiscard]] std::vector<Peer> announce(const TorrentFile& torrent_f);
}  // namespace peer
