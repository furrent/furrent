#include "download/downloader.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>

#include "download/util.hpp"
#include "hash.hpp"
#include "log/logger.hpp"

using fur::download::message::MessageKind;

namespace fur::download::downloader {
/// Should be greater than 10 seconds for a realistic torrent client but smaller
/// values result in quicker testing
const int UNCHOKE_TIMEOUT = 15;

DownloaderError from_socket_error(const socket::SocketError& err) {
  switch (err) {
    case socket::SocketError::Timeout:
      return DownloaderError::SocketTimeout;
    default:
      return DownloaderError::SocketOther;
  }
}

std::string display_downloader_error(const DownloaderError& err) {
  switch (err) {
    case DownloaderError::DifferentInfoHash:
      return "DifferentInfoHash";
    case DownloaderError::InvalidMessage:
      return "InvalidMessage";
    case DownloaderError::NoBitfield:
      return "NoBitfield";
    case DownloaderError::MissingPiece:
      return "MissingPiece";
    case DownloaderError::CorruptPiece:
      return "CorruptPiece";
    case DownloaderError::SocketTimeout:
      return "SocketTimeout";
    case DownloaderError::SocketOther:
      return "SocketOther";
    default:
      return "<invalid downloader error>";
  }
}

Downloader::Downloader(const TorrentFile& torrent, const Peer& peer)
    : torrent{torrent}, peer{peer} {}

Outcome<DownloaderError> Downloader::ensure_connected() {
  using Outcome = Outcome<DownloaderError>;

  auto logger = spdlog::get("custom");

  if (socket.has_value() && socket->is_open()) return Outcome::OK({});

  // Destroy any zombie socket and reset choked status (only really useful when
  // the socket is there but unhealthy. That is: `is_open` returns false)
  socket.reset();
  choked = true;

  // Construct the socket
  socket.emplace();

  // TCP connect
  auto maybe_connect =
      socket->connect(peer.ip, peer.port, std::chrono::seconds(5));
  if (!maybe_connect.valid()) {
    destroy_socket();
    return Outcome::ERROR(from_socket_error(maybe_connect.error()));
  }

  logger->debug("TCP connected with {}", peer.address());

  // BitTorrent handshake
  auto maybe_handshake = handshake();
  if (!maybe_handshake.valid()) return maybe_handshake;

  // `BitfieldMessage` is either the first message sent or the peer has no piece
  // to share. In the latter case, we're not interested in them and can drop the
  // connection.
  auto maybe_message = recv_message(std::chrono::seconds(5));
  if (!maybe_message.valid())
    return Outcome::ERROR(DownloaderError(maybe_message.error()));
  auto message = std::unique_ptr<Message>(maybe_message->release());

  if (message->kind() != MessageKind::Bitfield) {
    destroy_socket();
    return Outcome::ERROR(DownloaderError::NoBitfield);
  }
  auto& bitfield_message = dynamic_cast<BitfieldMessage&>(*message);
  bitfield.reset();
  bitfield.emplace(bitfield_message.bitfield.get_bytes(),
                   bitfield_message.bitfield.len);

  logger->debug("{} sent its bitfield", peer.address());

  auto maybe_unchoke = send_message(UnchokeMessage(), std::chrono::seconds(5));
  if (!maybe_unchoke.valid()) return maybe_unchoke;
  logger->debug("We unchoked {}", peer.address());

  auto maybe_interested =
      send_message(InterestedMessage(), std::chrono::seconds(5));
  if (!maybe_interested.valid()) return maybe_interested;
  logger->debug("We are interested in {}", peer.address());

  // Every connection starts chocked
  choked = true;

  // Now we need to wait to be unchoked by the peer. Only then can we start to
  // ask for pieces.
  while (choked) {
    maybe_message = recv_message(std::chrono::milliseconds(50));
    if (!maybe_message.valid())
      return Outcome::ERROR(DownloaderError(maybe_message.error()));
    message = std::unique_ptr<Message>(maybe_message->release());

    if (message->kind() == MessageKind::Unchoke) {
      choked = false;
    }
    // No need to do anything for all other messages
  }

  logger->debug("{} unchoked us", peer.address());

  return Outcome::OK({});
}

const TorrentFile& Downloader::get_torrent() const {
  return torrent;
}

const Peer& Downloader::get_peer() const {
  return peer;
}

// 1  for the length of the protocol identifier
// 19 for the protocol identifier itself
// 8  for the extensions bits
// 20 for the info hash
// 20 for the peer id
/// Length (in bytes) of a BitTorrent handshake.
const int HANDSHAKE_LENGTH = 1 + 19 + 8 + 20 + 20;
/// Offset (in bytes) from the beginning of a BitTorrent handshake for the
/// info-hash field.
const int INFO_HASH_OFFSET = 1 + 19 + 8;

Outcome<DownloaderError> Downloader::handshake() {
  using Outcome = Outcome<DownloaderError>;

  // Handshake message that we're going to build step-by-step
  std::vector<uint8_t> message;
  // Reserve some bytes so all calls to `insert` don't need to make heap
  // allocations, thus making them faster.
  message.reserve(HANDSHAKE_LENGTH);

  // Length of protocol identifier
  message.push_back(19);

  auto protocol = std::string("BitTorrent protocol");
  // Protocol identifier
  message.insert(message.end(), protocol.begin(), protocol.end());

  // 8 zeroes to indicate that we support no extensions
  message.resize(message.size() + 8);

  // Info hash
  message.insert(message.end(), torrent.info_hash.begin(),
                 torrent.info_hash.end());

  auto peerId = std::string("FUR-----------------");
  // Peer id
  message.insert(message.end(), peerId.begin(), peerId.end());

  // The message is ready, send it to the peer
  auto maybe_sent = socket->write(message, std::chrono::seconds(5));
  if (!maybe_sent.valid()) {
    destroy_socket();
    return Outcome::ERROR(from_socket_error(maybe_sent.error()));
  }

  // Read the response
  auto maybe_response =
      socket->read(HANDSHAKE_LENGTH, std::chrono::seconds(5));
  if (!maybe_response.valid()) {
    destroy_socket();
    return Outcome::ERROR(from_socket_error(maybe_response.error()));
  }
  auto response = *maybe_response;

  // Extract the info-hash from the response
  hash::hash_t response_info_hash;
  std::copy(response.begin() + INFO_HASH_OFFSET,
            response.begin() + INFO_HASH_OFFSET + sizeof(hash::hash_t{}),
            response_info_hash.begin());

  // In a BitTorrent handshake, the peer should respond with the same info-hash
  if (torrent.info_hash != response_info_hash) {
    destroy_socket();
    return Outcome::ERROR(DownloaderError::DifferentInfoHash);
  }

  auto logger = spdlog::get("custom");
  logger->debug("Hand shaken with {}", peer.address());

  return Outcome::OK({});
}

Result<Downloaded, DownloaderError> Downloader::try_download(const PieceDescriptor& task) {
  using Result = Result<Downloaded, DownloaderError>;

  auto logger = spdlog::get("custom");

  auto maybe_connected = ensure_connected();
  if (!maybe_connected.valid())
    return Result::ERROR(DownloaderError(maybe_connected.error()));

  // Peer doesn't have this piece
  if (!bitfield->get(task.index))
    return Result::ERROR(DownloaderError::MissingPiece);

  assert(!torrent.piece_hashes.empty());
  assert(torrent.piece_hashes.size() - 1 <= std::numeric_limits<long>::max());

  // The resulting piece
  std::vector<uint8_t> piece;

  auto piece_length = torrent.piece_length;
  // Might be shorter if this is the last piece
  if (static_cast<size_t>(task.index) == torrent.piece_hashes.size() - 1) {
    // Perform computation in `long` because `torrent.length` might be large but
    // then go back to a smaller `int` which should suffice.
    long before_this_piece =
        static_cast<long>(torrent.piece_hashes.size() - 1) *
        torrent.piece_length;
    assert(torrent.length >= before_this_piece);
    long l_piece_length = torrent.length - before_this_piece;
    assert(piece_length <= std::numeric_limits<int>::max());
    piece_length = static_cast<int>(l_piece_length);
  }

  piece.resize(piece_length);

  // How many bytes to demand in a `RequestMessage`. Should be 16KB.
  constexpr int BLOCK_SIZE = 16384;

  // How many blocks are there to download in total. Integer ceil division.
  const int blocks_total = (piece_length + BLOCK_SIZE - 1) / BLOCK_SIZE;
  // How many blocks have we requested so far.
  int blocks_requested = 0;
  // How many blocks have we received so far.
  int blocks_received = 0;

  // How many requested but unreceived blocks do we want to await at once
  constexpr int PIPELINE_SIZE_MAX = 5;

  // Dynamically updated to be longer after a Choke and shorter after an Unchoke
  auto timeout = std::chrono::seconds(5);

  // While the piece is not entirely downloaded
  while (blocks_received < blocks_total) {
    if (!choked) {
      while ((blocks_requested - blocks_received) < PIPELINE_SIZE_MAX &&
             blocks_requested < blocks_total) {
        // Might be shorter if this is the last block
        auto length = BLOCK_SIZE;
        if (blocks_requested == blocks_total - 1) {
          length = piece_length - blocks_requested * BLOCK_SIZE;
        }

        auto offset = blocks_requested * BLOCK_SIZE;

        auto maybe_sent =
            send_message(RequestMessage(task.index, offset, length),
                         std::chrono::seconds(5));
        if (!maybe_sent.valid()) {
          return Result::ERROR(DownloaderError(maybe_sent.error()));
        }

        blocks_requested++;
        logger->debug("Requested {} bytes at offset {} of piece {} from {}",
                      length, offset, task.index, peer.address());
      }
    }

    auto maybe_message = recv_message(timeout);
    if (!maybe_message.valid())
      return Result::ERROR(DownloaderError(maybe_message.error()));
    auto message = std::unique_ptr<Message>(maybe_message->release());

    switch (message->kind()) {
      case MessageKind::Choke:
        choked = true;
        // Use a slightly longer timeout to wait to be unchoked
        timeout = std::chrono::seconds(UNCHOKE_TIMEOUT);
        logger->debug("{} choked us", peer.address());
        break;
      case MessageKind::Unchoke:
        choked = false;
        // Reset timeout to the lower one
        timeout = std::chrono::seconds(5);
        logger->debug("{} unchoked us", peer.address());
        break;
      case MessageKind::Have: {
        // Nice, the peer has acquired a new piece that it can share
        auto& have_message = dynamic_cast<HaveMessage&>(*message);
        auto new_piece_index = have_message.index;
        bitfield->set(new_piece_index);
        logger->debug("{} now has piece {}", peer.address(), new_piece_index);
        break;
      }
      case MessageKind::Piece: {
        // There it is
        auto& piece_message = dynamic_cast<PieceMessage&>(*message);
        std::copy(piece_message.block.begin(), piece_message.block.end(),
                  piece.begin() + piece_message.begin);
        blocks_received++;
        logger->debug("{} sent us {} bytes at offset {} of piece {}",
                      peer.address(), piece_message.block.size(),
                      piece_message.begin, task.index);
        break;
      }
      default:;  // We can safely ignore other messages (hopefully)
    }
  }

  if (!hash::verify_piece(piece, torrent.piece_hashes[task.index])) {
    logger->debug("{} sent corrupt piece {}", peer.address(), task.index);
    return Result::ERROR(DownloaderError::CorruptPiece);
  }

  logger->debug("Piece {} completely downloaded from {}", task.index,
                peer.address());

  return Result::OK({task.index, std::move(piece)});
}

Outcome<DownloaderError> Downloader::send_message(const Message& msg,
                                                  timeout timeout) {
  auto outcome = socket->write(msg.encode(), timeout);
  if (outcome.valid()) {
    return Outcome<DownloaderError>::OK({});
  } else {
    destroy_socket();
    return Outcome<DownloaderError>::ERROR(from_socket_error(outcome.error()));
  }
}

Result<std::unique_ptr<Message>, DownloaderError> Downloader::recv_message(
    timeout timeout) {
  using Result = Result<std::unique_ptr<Message>, DownloaderError>;

  auto before_read_len = std::chrono::steady_clock::now();

  auto maybe_message_len_bytes = socket->read(4, timeout);
  if (!maybe_message_len_bytes.valid()) {
    destroy_socket();
    return Result::ERROR(from_socket_error(maybe_message_len_bytes.error()));
  }
  auto message_len_bytes = *maybe_message_len_bytes;

  auto message_len = decode_big_endian(
      std::array<uint8_t, 4>{message_len_bytes[0], message_len_bytes[1],
                             message_len_bytes[2], message_len_bytes[3]});

  // The time left is equal to the original timeout minus the time it took to
  // read the message length.
  timeout -= std::chrono::steady_clock::now() - before_read_len;

  auto maybe_rest = socket->read(message_len, timeout);
  if (!maybe_rest.valid()) {
    destroy_socket();
    return Result::ERROR(from_socket_error(maybe_rest.error()));
  }
  auto rest = *maybe_rest;

  std::vector<std::uint8_t> whole;
  whole.reserve(message_len_bytes.size() + rest.size());
  whole.insert(whole.end(), message_len_bytes.begin(), message_len_bytes.end());
  whole.insert(whole.end(), rest.begin(), rest.end());

  auto message = Message::decode(torrent, whole);
  if (!message.valid()) {
    destroy_socket();

    auto logger = spdlog::get("custom");
    logger->error("received invalid message: {}",
                  display_decode_error(message.error()));

    return Result::ERROR(DownloaderError::InvalidMessage);
  }

  return Result::OK(std::unique_ptr<Message>(message->release()));
}

void Downloader::destroy_socket() {
  try {
    socket->close();
  } catch (const std::exception& _) {
  }
  socket.reset();
}
}  // namespace fur::download::downloader