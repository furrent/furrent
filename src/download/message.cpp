#include "download/message.hpp"

#include <array>

#include "download/util.hpp"

namespace fur::download::message {
std::vector<uint8_t> Message::encode() const {
  // Collect bytes here
  std::vector<uint8_t> result;
  // We need to encode the payload first to be able to compute the message's
  // length.
  auto payload = encode_payload();
  // The length is 1 (for the message's ID) + whatever the payload's length is
  auto len = encode_big_endian(1 + payload.size());

  // Length goes first
  result.insert(result.end(), len.begin(), len.end());
  // Then message ID
  result.push_back(message_id());
  // Then payload
  result.insert(result.end(), payload.begin(), payload.end());

  return result;
}

std::optional<std::unique_ptr<Message>> Message::decode(
    const torrent::TorrentFile& torrent, const std::vector<uint8_t>& buf) {
  // Treat `KeepAliveMessage` differently because that's the only message
  // with no payload and such.
  if (buf == KeepAliveMessage().encode()) {
    return std::make_unique<KeepAliveMessage>();
  }

  // WARN: Do note that `buf.size()` is the length of the entire message
  // read from wire and does not match the length indicated by the first 4
  // bytes of the message itself, which is just 1 + the length of the payload.

  // Need at least 4 bytes for the message length and 1 for the message ID
  if (buf.size() < 5) return std::nullopt;

  auto id = buf[4];

  // Make a copy first, then shorten
  auto payload = std::vector(buf);
  // Skip over 4 bytes for the length and 1 for the message ID
  payload.erase(payload.begin(), payload.begin() + 5);

  switch (id) {
    case 0:
      // Payload should be empty for this message
      if (!payload.empty()) return std::nullopt;
      return std::make_unique<ChokeMessage>();
    case 1:
      // Payload should be empty for this message
      if (!payload.empty()) return std::nullopt;
      return std::make_unique<UnchokeMessage>();
    case 2:
      // Payload should be empty for this message
      if (!payload.empty()) return std::nullopt;
      return std::make_unique<InterestedMessage>();
    case 3:
      // Payload should be empty for this message
      if (!payload.empty()) return std::nullopt;
      return std::make_unique<NotInterestedMessage>();
    case 4:
      return HaveMessage::decode(payload);
    case 5:
      return BitfieldMessage::decode(torrent, payload);
    case 6:
      return RequestMessage::decode(payload);
    case 7:
      return PieceMessage::decode(payload);
    default:
      // Unknown message ID
      return std::nullopt;
  }
}

std::optional<std::unique_ptr<HaveMessage>> HaveMessage::decode(
    const std::vector<uint8_t>& buf) {
  // Payload should be exactly 4 bytes long: an unsigned 32 bits integer
  if (buf.size() != 4) return std::nullopt;

  auto index =
      decode_big_endian(std::array<uint8_t, 4>{buf[0], buf[1], buf[2], buf[3]});
  return std::make_unique<HaveMessage>(index);
}

std::vector<uint8_t> HaveMessage::encode_payload() const {
  auto array = encode_big_endian(index);

  // Payload is just the big-endian encoded index
  std::vector<uint8_t> result;
  result.reserve(4);
  result.insert(result.end(), array.begin(), array.end());

  return result;
}

std::optional<std::unique_ptr<BitfieldMessage>> BitfieldMessage::decode(
    const torrent::TorrentFile& torrent, const std::vector<uint8_t>& buf) {
  return std::make_unique<BitfieldMessage>(
      Bitfield(buf, torrent.piece_hashes.size()));
}

std::vector<uint8_t> BitfieldMessage::encode_payload() const {
  return bitfield.get_bytes();
}

std::optional<std::unique_ptr<RequestMessage>> RequestMessage::decode(
    const std::vector<uint8_t>& buf) {
  // Payload should be exactly 12 bytes long: 3x unsigned 32 bits integers
  if (buf.size() != 3 * 4) return std::nullopt;

  auto index =
      decode_big_endian(std::array<uint8_t, 4>{buf[0], buf[1], buf[2], buf[3]});
  auto begin =
      decode_big_endian(std::array<uint8_t, 4>{buf[4], buf[5], buf[6], buf[7]});
  auto length = decode_big_endian(
      std::array<uint8_t, 4>{buf[8], buf[9], buf[10], buf[11]});
  return std::make_unique<RequestMessage>(index, begin, length);
}

std::vector<uint8_t> RequestMessage::encode_payload() const {
  auto index_encoded = encode_big_endian(index);
  auto begin_encoded = encode_big_endian(begin);
  auto length_encoded = encode_big_endian(length);

  // Payload is just the big-endian encoded `index`, `begin` and `length` one
  // after another.
  std::vector<uint8_t> result;
  result.reserve(3 * 4);
  result.insert(result.end(), index_encoded.begin(), index_encoded.end());
  result.insert(result.end(), begin_encoded.begin(), begin_encoded.end());
  result.insert(result.end(), length_encoded.begin(), length_encoded.end());

  return result;
}

std::optional<std::unique_ptr<PieceMessage>> PieceMessage::decode(
    const std::vector<uint8_t>& buf) {
  // Payload should be at least 8 bytes long: 2x unsigned 32 bits integers
  if (buf.size() < 8) return std::nullopt;

  auto index =
      decode_big_endian(std::array<uint8_t, 4>{buf[0], buf[1], buf[2], buf[3]});
  auto begin =
      decode_big_endian(std::array<uint8_t, 4>{buf[4], buf[5], buf[6], buf[7]});

  // All remaining bytes compose the block
  auto block = buf;
  // Skip the first 8 bytes
  block.erase(block.begin(), block.begin() + 8);

  return std::make_unique<PieceMessage>(index, begin, block);
}

std::vector<uint8_t> PieceMessage::encode_payload() const {
  auto index_encoded = encode_big_endian(index);
  auto begin_encoded = encode_big_endian(begin);

  // Payload is just the big-endian encoded `index` and `begin` plus the bytes
  // from the block
  std::vector<uint8_t> result;
  result.reserve(2 * 4 + block.size());
  result.insert(result.end(), index_encoded.begin(), index_encoded.end());
  result.insert(result.end(), begin_encoded.begin(), begin_encoded.end());
  result.insert(result.end(), block.begin(), block.end());

  return result;
}
}  // namespace fur::download::message
