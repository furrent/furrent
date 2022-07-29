//
// Created by nicof on 28/07/22.
//

#pragma once

#include <string>
#include <memory>

#include "bencode_value.hpp"

namespace bencode {
  class BencodeParser {
   public:
    BencodeParser() = default;
    ~BencodeParser() = default;
    std::unique_ptr<BencodeValue> decode(std::string const &decoded);
    std::string encode(BencodeValue const &value);
  };
}