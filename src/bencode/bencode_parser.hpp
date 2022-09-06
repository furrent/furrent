#pragma once

#include <string>
#include <vector>

#include "bencode_value.hpp"
#include "util/result.hpp"

namespace fur::bencode {

/// Result of a parsing operation
typedef util::Result<std::unique_ptr<BencodeValue>>
    ParserResult;

class BencodeParser {

 private:
  /// The index of the current token used for decoding
  unsigned int _index{};
  /// The list of tokens to be decoded
  std::string _tokens;
  /// Private method used recursively to decode the bencode data
  ParserResult decode();
  /// Private method used to decode a bencode integer
  ParserResult decode_int();
  /// Private method used to decode a bencode string
  ParserResult decode_string();
  /// Private method used to decode a bencode list
  ParserResult decode_list();
  /// Private method used to decode a bencode dictionary
  ParserResult decode_dict();

 public:
  BencodeParser() = default;
  ~BencodeParser() = default;
  /// Parses a bencode string and returns a BencodeValue object, this is public
  /// and it is not called recursively, in fact it initialize the attributes
  /// _tokens and _index and then calls the private method decode()
  ParserResult decode(std::string const& decoded);

  /// Encodes a BencodeValue object into a bencode string
  std::string encode(BencodeValue const& value);
};

}  // namespace fur::bencode
