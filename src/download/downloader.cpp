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
Downloader::Downloader(const TorrentFile& torrent, const Peer& peer)
    : torrent{torrent}, peer{peer} {}

void Downloader::ensure_connected() {
  auto logger = spdlog::get("custom");

  if (socket.has_value() && socket->is_open()) return;

  // Destroy any zombie socket and reset choked status (only really useful when
  // the socket is there but unhealthy. That is: `is_open` returns false)
  socket.reset();
  choked = true;

  // Construct the socket
  socket.emplace();

  // TCP connect
  socket->connect(peer.ip, peer.port, std::chrono::milliseconds(2));
  logger->debug("TCP connected with {}", peer.address());

  // BitTorrent handshake
  handshake();

  // `BitfieldMessage` is either the first message sent or the peer has no piece
  // to share. In the latter case, we're not interested in them and can drop the
  // connection.
  auto message = recv_message(std::chrono::seconds(5));
  if (message->kind() != MessageKind::Bitfield) {
    throw std::runtime_error("no bitfield");
  }
  auto bitfield_message = dynamic_cast<BitfieldMessage&>(*message);
  bitfield.reset();
  bitfield.emplace(bitfield_message.bitfield.get_bytes(),
                   bitfield_message.bitfield.len);

  // Check that the bitfield has the correct length
  if (bitfield->len != torrent.piece_hashes.size()) {
    throw std::runtime_error(
        "peer sent a bitfield with a size that doesn't match the torrent");
  }

  logger->debug("{} sent its bitfield", peer.address());

  send_message(UnchokeMessage(), std::chrono::seconds(1));
  logger->debug("We unchoked {}", peer.address());
  send_message(InterestedMessage(), std::chrono::seconds(1));
  logger->debug("We are interested in {}", peer.address());

  // Every connection starts chocked
  choked = true;

  // Now we need to wait to be unchoked by the peer. Only then can we start to
  // ask for pieces.
  while (choked) {
    message = recv_message(std::chrono::seconds(11));
    if (message->kind() == MessageKind::Unchoke) {
      choked = false;
    }
    // No need to do anything for all other messages
  }
  logger->debug("{} unchoked us", peer.address());
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

void Downloader::handshake() {
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
  socket->write(message, std::chrono::milliseconds(100));

  // Read the response
  auto response =
      socket->read(HANDSHAKE_LENGTH, std::chrono::milliseconds(100));

  // Extract the info-hash from the response
  hash::hash_t response_info_hash;
  std::copy(response.begin() + INFO_HASH_OFFSET,
            response.begin() + INFO_HASH_OFFSET + sizeof(hash::hash_t{}),
            response_info_hash.begin());

  // In a BitTorrent handshake, the peer should respond with the same info-hash
  if (torrent.info_hash != response_info_hash) {
    throw std::runtime_error("bad handshake");
  }

  auto logger = spdlog::get("custom");
  logger->debug("Hand shaken with {}", peer.address());
}

std::optional<Result> Downloader::try_download(const Task& task) {
  auto logger = spdlog::get("custom");

  ensure_connected();

  // Peer doesn't have this piece
  if (!bitfield->get(task.index)) return std::nullopt;

  assert(task.index >= 0);
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
  auto timeout = std::chrono::seconds(2);

  try {
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

          send_message(RequestMessage(task.index, offset, length),
                       std::chrono::seconds(1));
          blocks_requested++;
          logger->debug("Requested {} bytes at offset {} of piece {} from {}",
                        length, offset, task.index, peer.address());
        }
      }

      auto message = recv_message(timeout);
      switch (message->kind()) {
        case MessageKind::Choke:
          choked = true;
          // Use a slightly longer timeout to wait to be unchoked
          timeout = std::chrono::seconds(11);
          logger->debug("{} choked us", peer.address());
          break;
        case MessageKind::Unchoke:
          choked = false;
          // Reset timeout to the lower one
          timeout = std::chrono::seconds(1);
          logger->debug("{} unchoked us", peer.address());
          break;
        case MessageKind::Have: {
          // Nice, the peer has acquired a new piece that it can share
          auto new_piece_index = dynamic_cast<HaveMessage&>(*message).index;
          bitfield->set(new_piece_index);
          logger->debug("{} now has piece {}", peer.address(), new_piece_index);
          break;
        }
        case MessageKind::Piece: {
          // There it is
          auto piece_message = dynamic_cast<PieceMessage&>(*message);
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
  } catch (const std::system_error& err) {
    logger->debug("Error downloading piece {} from {}: {}", task.index,
                  peer.address(), err.what());

    try {
      socket->close();
    } catch (const std::exception& _) {
      // `Downloader::ensure_connected` would think the socket is still healthy
      // if `socket->close()` somehow fails. Force it to recreate the socket.
      socket.reset();
    }

    return std::nullopt;
  }

  if (!hash::verify_piece(piece, torrent.piece_hashes[task.index])) {
    logger->debug("{} sent corrupt piece {}", peer.address(), task.index);
    return std::nullopt;
  }

  logger->debug("Piece {} completely downloaded from {}", task.index,
                peer.address());

  return Result{task.index, std::move(piece)};
}

void Downloader::send_message(const Message& msg, timeout timeout) {
  socket->write(msg.encode(), timeout);
}

std::unique_ptr<Message> Downloader::recv_message(timeout timeout) {
  auto before_read_len = std::chrono::steady_clock::now();

  auto message_len_bytes = socket->read(4, timeout);
  auto message_len = decode_big_endian(
      std::array<uint8_t, 4>{message_len_bytes[0], message_len_bytes[1],
                             message_len_bytes[2], message_len_bytes[3]});

  // The time left is equal to the original timeout minus the time it took to
  // read the message length.
  timeout -= std::chrono::steady_clock::now() - before_read_len;

  auto rest = socket->read(message_len, timeout);

  std::vector<std::uint8_t> whole;
  whole.reserve(message_len_bytes.size() + rest.size());
  whole.insert(whole.end(), message_len_bytes.begin(), message_len_bytes.end());
  whole.insert(whole.end(), rest.begin(), rest.end());

  auto message = Message::decode(torrent, whole);
  if (!message.has_value()) {
    throw std::runtime_error("unable to decode a message from the stream");
  }

  return std::move(message.value());
}

void Downloader::abort() {
  socket->close();
  socket = std::nullopt;
}
}  // namespace fur::download::downloader
