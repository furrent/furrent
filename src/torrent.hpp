#pragma once

#include <atomic>
#include <cstdint>
#include <deque>
#include <peer.hpp>
#include <random>
#include <string>
#include <types.hpp>
#include <vector>

#include "bencode/bencode_value.hpp"
#include "hash.hpp"

namespace fur {

/// Describes a file inside a torrent
struct File {
  /// Name of the file relative to the download folder,
  /// contains all subfolders
  std::vector<std::string> filepath;
  /// Number of bytes in the file
  int64_t length;

  // =======================================================

  /// @return filename in string form without base
  [[nodiscard]] std::string filename() const;
};

/// Represents a parsed .torrent file
struct TorrentFile {
  /// The URL used to announce ourselves to the tracker and fetch a list of
  /// peers
  std::string announce_url;
  /// The SHA1 hash of the "info" dict from the decoded .torrent file. Uniquely
  /// identifies this torrent to the tracker and other peers
  hash::hash_t info_hash{};
  /// SHA1 hashes, one for each piece of the shared file
  std::vector<hash::hash_t> piece_hashes;
  /// The length, in bytes, of each piece
  int64_t piece_length = 0;
  /// The length, in bytes, of the entire shared file
  int64_t length = 0;
  /// Total number of pieces
  int64_t pieces_count = 0;
  /// The name of the shared file
  std::string name;

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

/// Describes a subsection of a Piece, it is mapped to a single file
struct Subpiece {
  /// Path to the file this subpiece belongs to
  std::string filepath;
  /// Offset from the beginning of the file
  int64_t file_offset;
  /// Size in bytes
  int64_t len;
};

/// Describes a piece of a torrent with all the information
/// necessary to complete his download an saving on file
struct Piece {
  /// Global download index
  int64_t index;
  /// Mapping piece-files
  std::vector<Subpiece> subpieces;
};

enum class TorrentState {
  Loading,
  Downloading,
  Paused,
  Completed,
  Stopped,
  Error,
};

/// Completely describes a torrent in furrent
class Torrent {
  // Unique identifier for each torrent
  TorrentID _tid;
  /// Parsed .torrent file descriptor
  TorrentFile _descriptor;

  /// Peers where to ask for the pieces and interval time
  std::vector<peer::Peer> _peers;
  /// Hold the number of pieces successfully downloaded from each peer
  std::deque<std::atomic_int64_t> _peers_score;
  /// Next peers update interval time
  int64_t _update_interval;

 public:
  /// Current state of the torrent,
  /// this value can be changed concurrently
  std::atomic<TorrentState> state;

  /// Number of pieces downloaded and written to file.
  /// this value can be changed concurrently
  std::atomic_int64_t pieces_processed;

 public:
  /// Construct empty temporary torrent
  explicit Torrent();

  /// Construct a new Torrent
  /// @param tid unique id of the torrent
  /// @param descriptor parsed .torrent file descriptor
  Torrent(TorrentID tid, const TorrentFile& descriptor);

  /// Announces the client to the tracker and sets the list of peers
  void announce();

  /// Atomically increment score of a peer
  /// @param peer_index index of the peer to increment
  void atomic_add_peer_score(int64_t peer_index);

  /// Returns unique id
  [[nodiscard]] TorrentID tid() const;

  /// Returns the .torrent descriptor
  [[nodiscard]] const TorrentFile& descriptor() const;

  /// Returns the loaded peers
  [[nodiscard]] std::vector<peer::Peer> peers() const;

  /// Returns a peer distribution
  [[nodiscard]] std::discrete_distribution<int64_t> distribution() const;

  /// Generate all pieces of this torrent
  [[nodiscard]] std::vector<Piece> pieces() const;
};

}  // namespace fur
