#include "peer.hpp"

#include <memory>
#include <stdexcept>

#include "asio.hpp"
#include "bencode_parser.hpp"
#include "bencode_value.hpp"
#include "cpr/cpr.h"
#include "fmt/core.h"
#include "hash.hpp"

namespace fur::peer {
Peer::Peer(uint32_t ip, uint16_t port) : ip{ip}, port{port} {}

std::string Peer::address() const {
  return fmt::format("{}.{}.{}.{}:{}", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF,
                     (ip >> 8) & 0xFF, ip & 0xFF, port);
}

void Peer::connect() {
  using asio::ip::tcp;

  asio::io_context io_context;

  tcp::socket _socket(io_context);
  auto _ip = asio::ip::address_v4(this->ip);
  auto _port = asio::ip::port_type(this->port);
  _socket.connect(tcp::endpoint(_ip, _port));

  *socket = std::move(_socket);
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
