#pragma once

#include <string>

#include "bencode_value.hpp"

namespace bencode {
class BencodeParser {
 public:
  BencodeValue decode(const std::string& decoded);
  std::string encode(const BencodeValue& value);
};
}  // namespace bencode
