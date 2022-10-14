#include "download/message.hpp"

#include <array>

#include "download/util.hpp"

namespace fur::download::message {
std::string display_decode_error(const DecodeError& err) {
  switch (err) {
    case DecodeError::UnexpectedPayload:
      return "UnexpectedPayload";
    case DecodeError::InvalidHeader:
      return "InvalidHeader";
    case DecodeError::UnknownMessageID:
      return "UnknownMessageID";
    case DecodeError::InvalidPayloadLength:
      return "InvalidPayloadLength";
    default:
      return "<invalid decoding error>";
  }
}

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

Result<std::unique_ptr<Message>, DecodeError> Message::decode(
    const TorrentFile& torrent, const std::vector<uint8_t>& buf) {
  using Result = Result<std::unique_ptr<Message>, DecodeError>;

  // Treat `KeepAliveMessage` differently because that's the only message
  // with no payload and such.
  if (buf == KeepAliveMessage().encode()) {
    auto message = std::make_unique<KeepAliveMessage>();
    return Result::OK(std::move(message));
  }

  // WARN: Do note that `buf.size()` is the length of the entire message
  // read from wire and does not match the length indicated by the first 4
  // bytes of the message itself, which is just 1 + the length of the payload.

  // Need at least 4 bytes for the message length and 1 for the message ID
  if (buf.size() < 5) return Result::ERROR(DecodeError::InvalidHeader);

  auto id = buf[4];

  // Make a copy first, then shorten
  auto payload = std::vector(buf);
  // Skip over 4 bytes for the length and 1 for the message ID
  payload.erase(payload.begin(), payload.begin() + 5);

  // Simple, payload-less, messages
  if (id < 4) {
    // Payload should be empty for this message
    if (!payload.empty()) Result::ERROR(DecodeError::UnexpectedPayload);

    std::unique_ptr<Message> message;
    switch (id) {
      case 0: {
        message = std::make_unique<ChokeMessage>();
        break;
      }
      case 1: {
        message = std::make_unique<UnchokeMessage>();
        break;
      }
      case 2: {
        message = std::make_unique<InterestedMessage>();
        break;
      }
      case 3: {
        message = std::make_unique<NotInterestedMessage>();
        break;
      }
      default:;  // Unreachable
    }

    return Result::OK(std::move(message));
  } else if (id < 8) {
    DecodeError err;

    switch (id) {
      case 4: {
        auto message = HaveMessage::decode(payload);
        if (message.valid())
          return Result::OK(std::unique_ptr<Message>(message->release()));
        else
          err = message.error();
        break;
      }
      case 5: {
        auto message = BitfieldMessage::decode(torrent, payload);
        return Result::OK(std::move(message));
      }
      case 6: {
        auto message = RequestMessage::decode(payload);
        if (message.valid())
          return Result::OK(std::unique_ptr<Message>(message->release()));
        else
          err = message.error();
        break;
      }
      case 7: {
        auto message = PieceMessage::decode(payload);
        if (message.valid())
          return Result::OK(std::unique_ptr<Message>(message->release()));
        else
          err = message.error();
        break;
      }
      default:
        // Unreachable
        err = DecodeError(0);
    }

    return Result::ERROR(DecodeError(err));
  } else {
    // Unknown message ID
    return Result ::ERROR(DecodeError::UnknownMessageID);
  }
}

Result<std::unique_ptr<HaveMessage>, DecodeError> HaveMessage::decode(
    const std::vector<uint8_t>& buf) {
  using Result = Result<std::unique_ptr<HaveMessage>, DecodeError>;

  // Payload should be exactly 4 bytes long: an unsigned 32 bits integer
  if (buf.size() != 4) return Result::ERROR(DecodeError::InvalidPayloadLength);

  auto index =
      decode_big_endian(std::array<uint8_t, 4>{buf[0], buf[1], buf[2], buf[3]});

  auto message = std::make_unique<HaveMessage>(index);
  return Result::OK(std::move(message));
}

std::vector<uint8_t> HaveMessage::encode_payload() const {
  auto array = encode_big_endian(index);

  // Payload is just the big-endian encoded index
  std::vector<uint8_t> result;
  result.reserve(4);
  result.insert(result.end(), array.begin(), array.end());

  return result;
}

std::unique_ptr<BitfieldMessage> BitfieldMessage::decode(
    const TorrentFile& torrent, const std::vector<uint8_t>& buf) {
  return std::make_unique<BitfieldMessage>(
      Bitfield(buf, torrent.piece_hashes.size()));
}

std::vector<uint8_t> BitfieldMessage::encode_payload() const {
  return bitfield.get_bytes();
}

Result<std::unique_ptr<RequestMessage>, DecodeError> RequestMessage::decode(
    const std::vector<uint8_t>& buf) {
  using Result = Result<std::unique_ptr<RequestMessage>, DecodeError>;

  // Payload should be exactly 12 bytes long: 3x unsigned 32 bits integers
  if (buf.size() != 3 * 4)
    return Result::ERROR(DecodeError::InvalidPayloadLength);

  auto index =
      decode_big_endian(std::array<uint8_t, 4>{buf[0], buf[1], buf[2], buf[3]});
  auto begin =
      decode_big_endian(std::array<uint8_t, 4>{buf[4], buf[5], buf[6], buf[7]});
  auto length = decode_big_endian(
      std::array<uint8_t, 4>{buf[8], buf[9], buf[10], buf[11]});
  auto message = std::make_unique<RequestMessage>(index, begin, length);
  return Result::OK(std::move(message));
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

Result<std::unique_ptr<PieceMessage>, DecodeError> PieceMessage::decode(
    const std::vector<uint8_t>& buf) {
  using Result = Result<std::unique_ptr<PieceMessage>, DecodeError>;

  // Payload should be at least 8 bytes long: 2x unsigned 32 bits integers
  if (buf.size() < 8) return Result::ERROR(DecodeError::InvalidPayloadLength);

  auto index =
      decode_big_endian(std::array<uint8_t, 4>{buf[0], buf[1], buf[2], buf[3]});
  auto begin =
      decode_big_endian(std::array<uint8_t, 4>{buf[4], buf[5], buf[6], buf[7]});

  // All remaining bytes compose the block
  auto block = buf;
  // Skip the first 8 bytes
  block.erase(block.begin(), block.begin() + 8);

  auto message = std::make_unique<PieceMessage>(index, begin, block);
  return Result::OK(std::move(message));
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
