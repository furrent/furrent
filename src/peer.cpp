#include "peer.hpp"

#include <memory>
#include <regex>
#include <stdexcept>

#include "bencode/bencode_parser.hpp"
#include "bencode/bencode_value.hpp"
#include "cpr/cpr.h"
#include "fmt/core.h"
#include "hash.hpp"
#include "spdlog/spdlog.h"
#include "torrent.hpp"

namespace fur::peer {

std::string error_to_string(const PeerError error) {
  switch (error) {
    case PeerError::AnnounceError:
      return "can't announce to the tracker";
    case PeerError::ParserError:
      return "error during parsing of the file";
    default:
      return "unknown error";
  }
}

std::string Peer::address() const {
  return fmt::format("{}.{}.{}.{}:{}", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                     (ip >> 8) & 0xFF, ip & 0xFF, port);
}

Peer::Peer() : ip{0}, port{0} {}
Peer::Peer(uint32_t ip, uint16_t port) : ip{ip}, port{port} {}
Peer::Peer(const std::string& ip_s, uint16_t port) : ip{0}, port{port} {
  const std::regex ip_regex(R"(\s*(\d+)\.(\d+)\.(\d+)\.(\d+)\s*)");

  std::smatch match;
  std::regex_match(ip_s, match, ip_regex);

  if (match.empty()) {
    auto logger = spdlog::get("custom");
    logger->error("not a valid ip");
    throw std::invalid_argument("not a valid ip");
  }

  for (int i = 1; i <= 4; i++) {
    uint32_t octet = std::stoi(match[i]);
    ip |= octet << 8 * (4 - i);
  }
}

// Forward declare
PeerResult parse_tracker_response(const std::string& text);

PeerResult announce(const TorrentFile& torrent_f) {
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
    auto logger = spdlog::get("custom");
    logger->error("Could not announce to tracker");
    return PeerResult::ERROR(PeerError::AnnounceError);
  }
  return parse_tracker_response(res.text);
}

PeerResult parse_tracker_response(const std::string& text) {
  Announce result;

  bencode::BencodeParser parser;
  auto tree = parser.decode(text);
  if (!tree.valid()) {
    auto logger = spdlog::get("custom");
    logger->error("Bencode parser error: {}",
                  fur::bencode::error_to_string(tree.error()));
    return PeerResult::ERROR(PeerError::ParserError);
  }
  auto& dict = dynamic_cast<bencode::BencodeDict&>(*(*tree)).value();

  auto& interval = dynamic_cast<bencode::BencodeInt&>(*dict.at("interval"));
  result.interval = static_cast<int>(interval.value());

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

    result.peers.emplace_back(ip, port);
  }

  return PeerResult::OK(std::move(result));
}
}  // namespace fur::peer
