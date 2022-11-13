#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "download/bitfield.hpp"
#include "torrent.hpp"
#include "util/result.hpp"

using namespace fur::util;
using namespace fur::download::bitfield;

namespace fur::download::message {
enum class MessageKind {
  KeepAlive,
  Choke,
  Unchoke,
  Interested,
  NotInterested,
  Have,
  Bitfield,
  Request,
  Piece,
};

enum class DecodeError {
  /// Message has payload but didn't expect any
  UnexpectedPayload,
  /// Message has an invalid header. Meaning it's not a `KeepAliveMessage` and
  /// it's missing the length field or message id
  InvalidHeader,
  /// Not a known message type
  UnknownMessageID,
  /// Payload has invalid length, according the message's total length and
  /// message ID
  InvalidPayloadLength,
};

/// Virtual class for messages exchanged between BitTorrent clients.
/// They are all shaped like:
///   <length><id><payload>
class Message {
 public:
  virtual ~Message() = default;
  /// Encode this message to wire format. This is virtual because
  /// `KeepAliveMessage` uses a slightly different encoding that all other
  /// messages. That's the only subclass overriding `encode`.
  [[nodiscard]] virtual std::vector<uint8_t> encode() const;

  /// Try decoding a finite sequence of bytes into a `Message`. A return value
  /// `std::nullopt` indicates failure and receding communication with the peer
  /// is advised to avoid invalid state. The `TorrentFile` is currently only
  /// required because `BitfieldMessage` needs to know the piece count to
  /// decode itself nicely.
  [[nodiscard]] static Result<std::unique_ptr<Message>, DecodeError> decode(
      const TorrentFile& torrent, const std::vector<uint8_t>& buf);

  /// Returns the kind of this message. Used before `dynamic_cast`ing when the
  /// underlying type is not known beforehand.
  [[nodiscard]] virtual MessageKind kind() const = 0;

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
  [[nodiscard]] std::vector<uint8_t> encode() const override {
    return {0, 0, 0, 0};
  }

  [[nodiscard]] MessageKind kind() const override {
    return MessageKind::KeepAlive;
  }

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
 public:
  [[nodiscard]] MessageKind kind() const override { return MessageKind::Choke; }

 private:
  [[nodiscard]] uint8_t message_id() const override { return 0; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override {
    return {};
  }
};

/// Inform the peer that we're ready to accept more piece requests.
///   <length=1><id=1>
class UnchokeMessage final : public Message {
 public:
  [[nodiscard]] MessageKind kind() const override {
    return MessageKind::Unchoke;
  }

 private:
  [[nodiscard]] uint8_t message_id() const override { return 1; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override {
    return {};
  }
};

/// Inform the peer that we're interested in requesting pieces from it.
///   <length=1><id=2>
class InterestedMessage final : public Message {
 public:
  [[nodiscard]] MessageKind kind() const override {
    return MessageKind::Interested;
  }

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
 public:
  [[nodiscard]] MessageKind kind() const override {
    return MessageKind::NotInterested;
  }

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
  const int64_t index;

  explicit HaveMessage(int64_t index) : index{index} {}

  [[nodiscard]] static Result<std::unique_ptr<HaveMessage>, DecodeError> decode(
      const std::vector<uint8_t>& buf);

  [[nodiscard]] MessageKind kind() const override { return MessageKind::Have; }

 private:
  [[nodiscard]] uint8_t message_id() const override { return 4; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override;
};

/// Used by the peer to inform us of the pieces it has available for sharing.
///   <length=1+X><id=5><bitfield>
/// where X is the length of the bitfield.
class BitfieldMessage final : public Message {
 public:
  /// Bitfield representing the pieces available.
  const Bitfield bitfield;

  explicit BitfieldMessage(Bitfield bitfield) : bitfield{std::move(bitfield)} {}

  [[nodiscard]] static std::unique_ptr<BitfieldMessage> decode(
      const TorrentFile& torrent, const std::vector<uint8_t>& buf);

  [[nodiscard]] MessageKind kind() const override {
    return MessageKind::Bitfield;
  }

 private:
  [[nodiscard]] uint8_t message_id() const override { return 5; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override;
};

/// Ask the peer to send us a subset of the bytes in a piece.
///   <length=13><id=6><index><begin><length>
class RequestMessage final : public Message {
 public:
  /// Index of the piece.
  const int64_t index;
  /// Offset from the beginning of the piece.
  const int64_t begin;
  /// How many bytes we're asking. Typically 16KB.
  const int64_t length;

  RequestMessage(int64_t index, int64_t begin, int64_t length)
      : index{index}, begin{begin}, length{length} {}

  [[nodiscard]] static Result<std::unique_ptr<RequestMessage>, DecodeError>
  decode(const std::vector<uint8_t>& buf);

  [[nodiscard]] MessageKind kind() const override {
    return MessageKind::Request;
  }

 private:
  [[nodiscard]] uint8_t message_id() const override { return 6; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override;
};

/// A message containing a subset of the bytes from a piece.
///   <length=9+X><id=7><index><begin><block>
/// where X is the length of the block.
class PieceMessage final : public Message {
 public:
  /// Index of the piece.
  const int64_t index;
  /// Offset from the beginning of the piece.
  const int64_t begin;
  /// The actual bytes from the piece.
  const std::vector<uint8_t> block;

  PieceMessage(int64_t index, int64_t begin, std::vector<uint8_t> block)
      : index{index}, begin{begin}, block{std::move(block)} {}

  [[nodiscard]] static Result<std::unique_ptr<PieceMessage>, DecodeError>
  decode(const std::vector<uint8_t>& buf);

  [[nodiscard]] MessageKind kind() const override { return MessageKind::Piece; }

 private:
  [[nodiscard]] uint8_t message_id() const override { return 7; }
  [[nodiscard]] std::vector<uint8_t> encode_payload() const override;
};

// WARN: BitTorrent specifies a CancelMessage with ID 8, but we don't expect to
//  ever send or receive it

// WARN: BitTorrent specifies a PortMessage with ID 9, but we don't expect to
//  ever send or receive it
}  // namespace fur::download::message
