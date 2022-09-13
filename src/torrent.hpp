#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "bencode/bencode_value.hpp"
#include "hash.hpp"

namespace fur::torrent {
/// Represents a parsed .torrent file
struct TorrentFile {
 public:
  /// The URL used to announce ourselves to the tracker and fetch a list of
  /// peers
  std::string announce_url;
  /// The SHA1 hash of the "info" dict from the decoded .torrent file. Uniquely
  /// identifies this torrent to the tracker and other peers
  hash::hash_t info_hash;
  /// SHA1 hashes, one for each piece of the shared file
  std::vector<hash::hash_t> piece_hashes;
  /// The length, in bytes, of each piece
  int piece_length;
  /// The length, in bytes, of the entire shared file
  long length;
  /// The name of the shared file
  std::string name;
  /// Shared output file stream for results
  std::shared_ptr<std::ofstream> stream_ptr;

  /// Construct an empty TorrentFile instance
  explicit TorrentFile() = default;

  /// Construct an instance of TorrentFile given a bencode::BencodeValue which
  /// is assumed to be the parsed .torrent file
  explicit TorrentFile(const bencode::BencodeValue& tree);
};
}  // namespace fur::torrent
