//
// Created by nicof on 28/07/22.
//

#pragma once

#include <string>

#include "bencode_value.hpp"

namespace bencode {
  class BencodeParser {
   public:
    BencodeParser() = default;
    ~BencodeParser() = default;
    BencodeValue decode(std::string &decoded);
    std::string encode(BencodeValue &value);
  };
}