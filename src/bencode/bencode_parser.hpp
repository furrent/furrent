#pragma once

#include <string>
#include <vector>

#include "bencode_value.hpp"
#include "util/result.hpp"

namespace fur::bencode {

/// Possible errors that can occur during the parsing of a bencoded string
enum class BencodeParserError{
  /// The bencoded string is not valid
  InvalidString,
  /// A integer was not in the form ['i', 'number', 'e']
  IntFormat,
  /// The integer was not a valid integer
  IntValue,
  /// A string was not in the form ['length', ':', 'string']
  StringFormat,
  /// A string has a length that is not a valid integer
  StringLength,
  /// A list was not in the form ['l', 'bencode_value', ... , 'e']
  ListFormat,
  /// A dictionary was not in the form ['d', 'bencode_value', ... , 'e']
  DictFormat,
  /// A dictionary key was not a string
  DictKey,
  /// The keys of the dictionary were not in lexicographical order
  DictKeyOrder,
};

class BencodeParser {

  /// Result of a parsing operation
  typedef util::Result<std::unique_ptr<BencodeValue>, BencodeParserError> Result;

 private:
  /// The index of the current token used for decoding
  unsigned int _index{};
  /// The list of tokens to be decoded
  std::string _tokens;
  /// Private method used recursively to decode the bencode data
  Result decode();
  /// Private method used to decode a bencode integer
  Result decode_int();
  /// Private method used to decode a bencode string
  Result decode_string();
  /// Private method used to decode a bencode list
  Result decode_list();
  /// Private method used to decode a bencode dictionary
  Result decode_dict();

 public:
  BencodeParser() = default;
  ~BencodeParser() = default;
  /// Parses a bencode string and returns a BencodeValue object, this is public
  /// and it is not called recursively, in fact it initialize the attributes
  /// _tokens and _index and then calls the private method decode()
  Result decode(std::string const& decoded);

  /// Encodes a BencodeValue object into a bencode string
  std::string encode(BencodeValue const& value);
};

}  // namespace fur::bencode
