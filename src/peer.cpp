#include "peer.hpp"

#include <memory>
#include <stdexcept>

#include "bencode_parser.hpp"
#include "bencode_value.hpp"
#include "cpr/cpr.h"
#include "fmt/core.h"
#include "hash.hpp"

namespace fur::peer {
std::string Peer::address() const {
  return fmt::format("{}.{}.{}.{}:{}", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                     (ip >> 8) & 0xFF, ip & 0xFF, port);
}

// Forward declare
AnnounceResult parse_tracker_response(const std::string& text);

AnnounceResult announce(const torrent::TorrentFile& torrent_f) {
  auto res = cpr::Get(cpr::Url{torrent_f.announce_url},
                      cpr::Parameters{
                          {"info_hash", hash::hash_to_str(torrent_f.info_hash)},
                          {"peer_id", "FUR-----------------"},
                          {"port", "6881"},
                          {"uploaded", "0"},
                          {"downloaded", "0"},
                          {"compact", "0"},
                          {"left", std::to_string(torrent_f.length)},
                      });

  if (res.status_code == 0 || res.status_code >= 400) {
    throw std::runtime_error("could not announce to tracker");
  }

  return parse_tracker_response(res.text);
}

AnnounceResult parse_tracker_response(const std::string& text) {
  AnnounceResult result;

  bencode::BencodeParser parser;
  std::unique_ptr<bencode::BencodeValue> tree = parser.decode(text);
  auto& dict = dynamic_cast<bencode::BencodeDict&>(*tree).value();

  auto& interval = dynamic_cast<bencode::BencodeInt&>(*dict.at("interval"));
  result.interval = interval.value();

  auto& peers = dynamic_cast<bencode::BencodeString&>(*dict.at("peers"));
  for (auto it = peers.value().begin(); it < peers.value().end(); it += 6) {
    // Each byte is an octet
    uint32_t ip = 0;
    ip |= *(it++) << 24;
    ip |= *(it++) << 16;
    ip |= *(it++) << 8;
    ip |= *(it++);

    // Big endian
    uint16_t port = 0;
    port |= *(it++) << 8;
    port |= *(it++);

    result.peers.push_back(Peer{ip, port});
  }

  return result;
}
}  // namespace fur::peer
