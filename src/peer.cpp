#include "peer.hpp"

#include "cpr/cpr.h"
#include "fmt/core.h"
#include "hash.hpp"

namespace peer {
std::string Peer::address() const {
  return fmt::format("{}.{}.{}.{}:{}", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                     (ip >> 8) & 0xFF, ip & 0xFF, port);
}

// TODO Add test
std::vector<Peer> announce(const TorrentFile& torrent_f) {
  auto res = cpr::Get(cpr::Url{torrent_f.announce_url},
                      cpr::Parameters{
                          {"info_hash", hash_to_str(torrent_f.info_hash)},
                          {"peer_id", "FUR-----------------"},
                          {"port", "6881"},
                          {"uploaded", "0"},
                          {"downloaded", "0"},
                          {"compact", "0"},
                          {"left", std::to_string(torrent_f.length)},
                      });
  // TODO Parse tracker response
  return std::vector<Peer>{};
}
}  // namespace peer
