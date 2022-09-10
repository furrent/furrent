#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "download/bitfield.hpp"
#include "download/message.hpp"
#include "download/socket.hpp"
#include "peer.hpp"
#include "tfriend_fw.hpp"
#include "torrent.hpp"
#include "util/result.hpp"

using namespace fur::util;
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
struct Downloaded {
  int index;
  std::vector<uint8_t> content;
};

namespace fur::download::downloader {
enum class DownloaderError {
  /// The peer sent back a differnt info hash when handshaking
  DifferentInfoHash,
  /// The peer sent a bitfield with a length that doesn't match the torrent
  InvalidBitfieldLength,
  /// Error decoding a BitTorrent TCP protocol message
  InvalidMessage,
  /// The peer doesn't have the requested piece
  MissingPiece,
  /// The piece was correctly downloaded but doesn't match the expected hash
  CorruptPiece,
  /// The socket timed out
  SocketTimeout,
  /// The socket experienced some other, generic, error
  SocketOther,
};

DownloaderError from_socket_error(const socket::SocketError& err);

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
  [[nodiscard]] Result<Downloaded, DownloaderError> try_download(const Task&);

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
  Outcome<DownloaderError> ensure_connected();
  /// Performs the BitTorrent handshake.
  Outcome<DownloaderError> handshake();

  Outcome<DownloaderError> send_message(const Message& msg, timeout timeout);
  Result<std::unique_ptr<Message>, DownloaderError> recv_message(
      timeout timeout);

  /// Should be called after any socket error to make sure that it is re-created
  /// upon new operations.
  void destroy_socket();

  // Befriend this class so the unit tests are able to access private members.
  friend TestingFriend;
};
}  // namespace fur::download::downloader
