//
// Created by nicof on 28/07/22.
//

#pragma once

#include <string>
#include <memory>

#include "bencode_value.hpp"

namespace fur::bencode {

/// Contains the structure for decoding and encoding bencode data
class BencodeParser {
 public:
  BencodeParser() = default;
  ~BencodeParser() = default;
  /// Parses a bencode string and returns a BencodeValue object
  std::unique_ptr<BencodeValue> decode(std::string const &decoded);
  /// Encodes a BencodeValue object into a bencode string
  std::string encode(BencodeValue const &value);
};

} // namespace fur::bencode

