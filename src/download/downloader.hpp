#pragma once

#include <memory>
#include <optional>

#include "download/bitfield.hpp"
#include "download/message.hpp"
#include "download/socket.hpp"
#include "peer.hpp"
#include "tfriend_fw.hpp"
#include "torrent.hpp"

using namespace fur::peer;
using namespace fur::torrent;
using namespace fur::download::socket;
using namespace fur::download::message;

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

  /// True iff we are choked by the peer. This means that we shouldn't try to
  /// send any `RequestMessage` before they would simply be discarded. Every
  /// BitTorrent connection is initially chocked until proved otherwise. Because
  /// a `Downloader` is not re-created when a connection drops but recycled,
  /// we should take care to reset this to `true`.
  bool choked = true;
  /// Tracks what pieces this peer has available for sharing. Should be reset to
  /// `std::nullopt` when a connection drops and is later recycled.
  std::optional<Bitfield> bitfield;

  /// Ensures that the `socket` is present and in good health (not dropped,
  /// timed out and such). Should always call this method first, before
  /// accessing the socket.
  void ensure_connected();
  /// Performs the BitTorrent handshake.
  void handshake();

  void send_message(const Message& msg, timeout timeout);
  std::optional<std::unique_ptr<Message>> recv_message(timeout timeout);

  // Befriend this class so the unit tests are able to access private members.
  friend TestingFriend;
};
}  // namespace fur::download::downloader
