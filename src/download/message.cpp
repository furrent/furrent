#include "download/message.hpp"

#include <array>

/// Takes a 32 bits unsigned integer and encodes it into a 4 bytes array.
std::array<uint8_t, 4> encode_big_endian(uint32_t n) {
  std::array<uint8_t, 4> result{};
  for (int i = 3; i >= 0; i--) {
    result[i] = n & 0xFF;
    n >>= 8;
  }
  return result;
}

/// Decodes a 32 bits unsigned integer from a 4 bytes array.
uint32_t decode_big_endian(const std::array<uint8_t, 4>& buf) {
  uint32_t result = 0;
  for (int i = 0; i <= 3; i++) {
    result |= buf[i] << (8 * (3 - i));
  }
  return result;
}

std::vector<uint8_t> Message::encode() {
  // Collect bytes here
  std::vector<uint8_t> result;
  // We need to encode the payload first to be able to compute the message's
  // length.
  auto payload = encode_payload();
  // The length is 1 (for the message's ID) + whatever the payload's length is
  auto len = encode_big_endian(1 + payload.size());

  // Length goes first
  result.insert(result.begin(), len.begin(), len.end());
  // Then message ID
  result.push_back(message_id());
  // Then payload
  result.insert(result.end(), payload.begin(), payload.end());

  return result;
}

std::optional<std::unique_ptr<Message>> Message::decode(
    const std::vector<uint8_t>& buf) {
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
    case 4: {
      return HaveMessage::decode(payload);
    }
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
  result.insert(result.begin(), array.begin(), array.end());

  return result;
}
