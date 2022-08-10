#pragma once

#include <string>
#include <vector>

#include "bencode_value.hpp"

namespace fur::bencode {

/// Contains the structure for decoding and encoding bencode data
class BencodeParser {
 private:
  /// The index of the current token used for decoding
  unsigned int _index;
  /// Return the tokens of a bencode string
  std::vector<std::string> tokenizer(const std::string& encoded);
  /// Private method used recursively to decode the bencode data
  std::unique_ptr<BencodeValue> decode(const std::vector<std::string>& tokens);
  /// Private method used to decode a bencode integer
  std::unique_ptr<BencodeValue> decode_int(
      const std::vector<std::string>& tokens);
  /// Private method used to decode a bencode string
  std::unique_ptr<BencodeValue> decode_string(
      const std::vector<std::string>& tokens);
  /// Private method used to decode a bencode list
  std::unique_ptr<BencodeValue> decode_list(
      const std::vector<std::string>& tokens);
  /// Private method used to decode a bencode dictionary
  std::unique_ptr<BencodeValue> decode_dict(
      const std::vector<std::string>& tokens);

 public:
  BencodeParser() = default;
  ~BencodeParser() = default;
  /// Parses a bencode string and returns a BencodeValue object, this is public
  /// and it is not called recursively, in fact use the function tokenizer to
  /// get the tokens of the bencode string and the put the _index to 0 to start
  /// the decoding process
  std::unique_ptr<BencodeValue> decode(std::string const& decoded);

  /// Encodes a BencodeValue object into a bencode string
  std::string encode(BencodeValue const& value);
};

}  // namespace fur::bencode
