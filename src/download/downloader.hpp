#pragma once

#include <optional>

#include "asio.hpp"
#include "peer.hpp"
#include "tfriend_fw.hpp"
#include "torrent.hpp"

using namespace fur::peer;
using namespace fur::torrent;

/// The `TorrentFile` is not required because any given `Downloader` is already
/// bound to a specific `TorrentFile` and has a reference to it.
// TODO Remove once the real struct is merged
struct Task {
  int index;
};

/// A downloaded piece for a torrent file.
// TODO Remove once the real struct is merged
struct Result {
  int index;
  std::vector<uint8_t> content;
};

namespace fur::download::downloader {
/// The type used for TCP communication
using Socket = asio::ip::tcp::socket;

/// Handles downloading of torrent pieces. Must be initialized with a
/// `TorrentFile` and a `Peer` discovered from that same torrent. This type is
/// intrinsically not copyable because it embeds an ASIO socket.
class Downloader {
 public:
  /// Construct a new `Downloader`. No TCP socket is established at this time.
  explicit Downloader(const TorrentFile& torrent, const Peer& peer);

  /// Attempt downloading a piece using this `Downloader`. The function tries
  /// it best not to throw any exception (unless something truly exceptional
  /// happens). You can assume that any ordinary error will result in a
  /// `std::nullopt` being returned. Errors such as:
  ///  - This peer not having the requested piece available
  ///  - The connection timing out
  ///  - The downloaded piece being corrupt
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
}  // namespace fur::download::downloader
