#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace fur::download::message {
/// Virtual class for messages exchanged between BitTorrent clients.
/// They are all shaped like:
///   <length><id><payload>
class Message {
 public:
  ~Message() = default;
  /// Encode this message to wire format. This is virtual because
  /// `KeepAliveMessage` uses a slightly different encoding that all other
  /// messages. That's the only subclass overriding `encode`.
  [[nodiscard]] virtual std::vector<uint8_t> encode();

  /// Try decoding a finite sequence of bytes into a `Message`. A return value
  /// `std::nullopt` indicates failure and receding communication with the peer
  /// is advised to avoid invalid state.
  [[nodiscard]] static std::optional<std::unique_ptr<Message>> decode(
      const std::vector<uint8_t>& buf);

 private:
  /// Returns the ID of this message type. Used for encoding only.
  [[nodiscard]] virtual uint8_t message_id() const = 0;
  /// Encodes the payload section of this message. Overridden by each specific
  /// message type.
  [[nodiscard]] virtual std::vector<uint8_t> encode_payload() const = 0;
};

/// Periodically sent to keep the socket alive.
///   <length=0> (no id)
/// Because this is the only message with no ID, we override the `encode`
/// function in its entirety, instead of only overriding `encode_payload`.
class KeepAliveMessage final : public Message {
 public:
  /// This message type is encoded slightly differently than other messages
  /// (there's no ID) so we use a custom encoding function.
  [[nodiscard]] std::vector<uint8_t> encode() override { return {0, 0, 0, 0}; }

 private:
  /// Never really called but must be implemented to make `KeepAliveMessage` a
  /// complete type.
  [[nodiscard]] uint8_t message_id() const override { return 0; }
  /// Never really called but must be implemented to make `KeepAliveMessage` a
  /// complete type.
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override {
    return {};
  }
};

/// Inform the peer that we're not going to accept any more requests until
/// unchoked.
///   <length=1><id=0>
class ChokeMessage final : public Message {
 private:
  [[nodiscard]] uint8_t message_id() const override { return 0; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override {
    return {};
  }
};

/// Inform the peer that we're ready to accept more piece requests.
///   <length=1><id=1>
class UnchokeMessage final : public Message {
 private:
  [[nodiscard]] uint8_t message_id() const override { return 1; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override {
    return {};
  }
};

/// Inform the peer that we're interested in requesting pieces from it.
///   <length=1><id=2>
class InterestedMessage final : public Message {
 private:
  [[nodiscard]] uint8_t message_id() const override { return 2; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override {
    return {};
  }
};

/// Inform the peer that we're no longer interested in requesting pieces from
/// it.
///   <length=1><id=3>
class NotInterestedMessage final : public Message {
 private:
  [[nodiscard]] uint8_t message_id() const override { return 3; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override {
    return {};
  }
};

/// Inform the peer that we have acquired a new piece.
///   <length=5><id=4><piece-index>
class HaveMessage final : public Message {
 public:
  /// Index of the newly acquired piece.
  const uint32_t index;

  explicit HaveMessage(uint32_t index) : index{index} {}

  [[nodiscard]] static std::optional<std::unique_ptr<HaveMessage>> decode(
      const std::vector<uint8_t>& buf);

 private:
  [[nodiscard]] uint8_t message_id() const override { return 4; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override;
};
}  // namespace fur::download::message
