#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "bencode/bencode_value.hpp"
#include "hash.hpp"

namespace fur::torrent {

/// Describes a file inside a torrent
struct File {
  /// Name of the file relative to the download folder,
  /// contains all subfolders
  std::vector<std::string> filepath;
  /// Number of bytes in the file
  size_t length;
};

/// Represents a parsed .torrent file
struct TorrentFile {
  /// The URL used to announce ourselves to the tracker and fetch a list of
  /// peers
  std::string announce_url;
  /// The SHA1 hash of the "info" dict from the decoded .torrent file. Uniquely
  /// identifies this torrent to the tracker and other peers
  hash::hash_t info_hash;
  /// SHA1 hashes, one for each piece of the shared file
  std::vector<hash::hash_t> piece_hashes;
  /// The length, in bytes, of each piece
  size_t piece_length;
  /// The length, in bytes, of the entire shared file
  size_t length;
  /// Total number of pieces
  size_t pieces_count; 
  /// The name of the shared file
  std::string name;

  /// Shared output file stream for results
  std::shared_ptr<std::ofstream> stream_ptr;
  /// Name of the folder containing all torrent files
  std::string folder_name;
  /// Describe the structure of the file
  std::vector<File> files;

  /// Construct an empty TorrentFile instance
  explicit TorrentFile() = default;

  /// Construct an instance of TorrentFile given a bencode::BencodeValue which
  /// is assumed to be the parsed .torrent file
  explicit TorrentFile(const bencode::BencodeValue& tree);
};
}  // namespace fur::torrent
