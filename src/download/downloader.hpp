#pragma once

#include <optional>

#include "asio.hpp"
#include "peer.hpp"
#include "tfriend_fw.hpp"
#include "torrent.hpp"

using namespace fur::peer;
using namespace fur::torrent;

// TODO Remove once the real struct is merged
struct Task {
  int index;
};

// TODO Remove once the real struct is merged
struct Result {
  int index;
  std::vector<uint8_t> content;
};

namespace fur::downloader {
/// The type used for TCP communication
using Socket = asio::ip::tcp::socket;

/// Handles downloading of torrent pieces. Must be initialized with a
/// `TorrentFile` and a `Peer` discovered from that same torrent. This type is
/// intrinsically not copyable because it embeds an ASIO socket.
class Downloader {
 public:
  /// Construct a new `Downloader`. No TCP socket is established at this time.
  explicit Downloader(const TorrentFile& torrent, const Peer& peer);

  [[nodiscard]] std::optional<Result> try_download(const Task&);

 private:
  const TorrentFile& torrent;
  const Peer& peer;

  /// Socket that this `Downloader` has established with a `Peer`. This is
  /// lazily initialized when needed and kept in good health thanks to
  /// `ensure_connected`.
  std::optional<Socket> socket;

  /// Ensures that the `socket` is present and in good health (not dropped,
  /// timed out and such). Should always call this method first, before
  /// accessing the socket.
  void ensure_connected();
  void handshake();

  // Befriend this class so the unit tests are able to access private members.
  friend TestingFriend;
};
}  // namespace fur::downloader
