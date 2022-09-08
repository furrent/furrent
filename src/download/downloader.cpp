#include "download/downloader.hpp"

#include <array>
#include <cstdint>
#include <stdexcept>

#include "asio.hpp"
#include "download/util.hpp"

using fur::download::message::MessageKind;

namespace fur::download::downloader {
Downloader::Downloader(const TorrentFile& torrent, const Peer& peer)
    : torrent{torrent}, peer{peer} {}

void Downloader::ensure_connected() {
  if (socket.has_value() && socket->is_open()) return;

  // Destroy any zombie socket
  socket.reset();
  // Construct the socket
  socket.emplace();

  // TCP connect
  socket->connect(peer.ip, peer.port, std::chrono::milliseconds(2));
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

  send_message(UnchokeMessage(), std::chrono::seconds(1));
  send_message(InterestedMessage(), std::chrono::seconds(1));

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
}

std::optional<Result> Downloader::try_download(const Task& task) {
  // Ask for 16KB
  send_message(RequestMessage(task.index, 0, 16384), std::chrono::seconds(1));

  std::optional<std::vector<uint8_t>> block;

  auto timeout = std::chrono::seconds(1);
  while (!block.has_value()) {
    auto message = recv_message(timeout);

    switch (message->kind()) {
      case MessageKind::Choke:
        choked = true;
        // Use a slightly longer timeout to wait to be unchoked
        timeout = std::chrono::seconds(11);
        break;
      case MessageKind::Unchoke:
        choked = false;
        // Reset timeout to the lower one
        timeout = std::chrono::seconds(1);
        break;
      case MessageKind::Have:
        // Nice, the peer has acquired a new piece that it can share
        bitfield->set(dynamic_cast<HaveMessage&>(*message).index);
        break;
      case MessageKind::Piece:
        // There it is
        block = dynamic_cast<PieceMessage&>(*message).block;
        break;
      default:;  // We can safely ignore other messages (hopefully)
    }
  }
  // If we got to this line, that means we received our data so the test passes
  // TODO Download more than just the first 16KB
  return Result{0, std::move(block.value())};
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