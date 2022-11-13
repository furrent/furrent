#pragma once

#include <string>
#include <vector>

#include "bencode_value.hpp"
#include "util/result.hpp"

namespace fur::bencode {

enum class BencodeParserError {
  /// The bencoded string is not valid, the most general error that can occur
  /// typically where the string is not well formed
  InvalidString,
  /// A integer was not in the form ['i', 'number', 'e']
  IntFormat,
  /// The integer was not a valid integer
  IntValue,
  /// A string was not in the form ['length', ':', 'string']
  StringFormat,
  /// A list was not in the form ['l', 'bencode_value', ... , 'e']
  ListFormat,
  /// A dictionary was not in the form ['d', 'bencode_value', ... , 'e']
  DictFormat,
  /// A dictionary key was not a string
  DictKey,
  /// The keys of the dictionary were not in lexicographical order
  DictKeyOrder
};

/// Function to translate a BencodeParserError into a string
std::string error_to_string(BencodeParserError error);

/// Result of a parsing operation
using BencodeResult =
    util::Result<std::unique_ptr<BencodeValue>, BencodeParserError>;

class BencodeParser {
 private:
  /// The index of the current token used for decoding
  int64_t _index{};
  /// The list of tokens to be decoded
  std::string _tokens;
  /// Private method used recursively to decode the bencode data
  BencodeResult decode();
  /// Private method used to decode a bencode integer
  BencodeResult decode_int();
  /// Private method used to decode a bencode string
  BencodeResult decode_string();
  /// Private method used to decode a bencode list
  BencodeResult decode_list();
  /// Private method used to decode a bencode dictionary
  BencodeResult decode_dict();

 public:
  BencodeParser() = default;
  ~BencodeParser() = default;
  /// Parses a bencode string and returns a BencodeValue object, this is public
  /// and it is not called recursively, in fact it initialize the attributes
  /// _tokens and _index and then calls the private method decode()
  BencodeResult decode(std::string const& decoded);

  /// Encodes a BencodeValue object into a bencode string
  static std::string encode(BencodeValue const& value);
};

}  // namespace fur::bencode
