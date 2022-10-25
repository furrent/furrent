#include "bencode_parser.hpp"

#include <regex>

namespace fur::bencode {
std::string error_to_string(const BencodeParserError error) {
  switch (error) {
    case BencodeParserError::InvalidString:
      return "InvalidString";
    case BencodeParserError::IntFormat:
      return "IntFormat";
    case BencodeParserError::IntValue:
      return "IntValue";
    case BencodeParserError::StringFormat:
      return "StringFormat";
    case BencodeParserError::ListFormat:
      return "ListFormat";
    case BencodeParserError::DictFormat:
      return "DictFormat";
    case BencodeParserError::DictKey:
      return "DictKey";
    case BencodeParserError::DictKeyOrder:
      return "DictKeyOrder";
    default:
      return "<invalid parser error>";
  }
}

std::string BencodeParser::encode(const BencodeValue& value) {
  return value.to_string();
}

auto BencodeParser::decode(const std::string& decoded) -> BencodeResult {
  _tokens = decoded;
  _index = 0;  // Reset the index
  auto r = decode();
  if (r.valid() && _index != _tokens.size()) {
    // If the result is not an error and the string was not fully parsed
    return BencodeResult::ERROR(BencodeParserError::InvalidString);
  }
  return r;
}

// Given a vector of tokens, returns a BencodeValue object
auto BencodeParser::decode() -> BencodeResult {
  if (!_tokens.empty()) {
    auto token = _tokens[_index];
    if (token == 'i') {
      return BencodeParser::decode_int();
    } else if (token <= '9' && token >= '0') {
      return BencodeParser::decode_string();
    } else if (token == 'l') {
      return BencodeParser::decode_list();
    } else if (token == 'd') {
      return BencodeParser::decode_dict();
    }
  }
  return BencodeResult::ERROR(BencodeParserError::InvalidString);
}

auto BencodeParser::decode_int() -> BencodeResult {
  // The token must be in the form ['i', 'number', 'e']
  if (_tokens.size() - _index < 3) {
    return BencodeResult::ERROR(BencodeParserError::IntFormat);
  }
  // Skip the 'i' token already checked before entering this function
  _index++;
  // Decoding the integer
  std::string integer{};
  while (_index < _tokens.size() && _tokens[_index] != 'e') {
    integer += _tokens[_index];
    _index++;
  }
  if (_index == _tokens.size()) {
    // No space for the 'e' token
    return BencodeResult::ERROR(BencodeParserError::IntFormat);
  } else if (_tokens[_index] != 'e') {
    // Missing 'e' at the end of the integer
    return BencodeResult::ERROR(BencodeParserError::IntFormat);
  } else if (integer == "-0") {
    // The "-0" is not a valid integer
    return BencodeResult::ERROR(BencodeParserError::IntValue);
  } else if (!std::regex_match(integer, std::regex("^-?\\d+$"))) {
    // Check if the integer is a string of digits with sign
    return BencodeResult::ERROR(BencodeParserError::IntValue);
  }
  // Skip 'e' at the end
  _index++;
  return BencodeResult::OK(std::make_unique<BencodeInt>(std::stol(integer)));
}

auto BencodeParser::decode_string() -> BencodeResult {
  // The token must be in the form ['length', ':', 'string']
  if (_tokens.size() - _index < 3) {
    return BencodeResult::ERROR(BencodeParserError::StringFormat);
  }
  // Calculate the string length until the ':' token
  std::string len{};
  while (_index < _tokens.size() && _tokens[_index] != ':') {
    len += _tokens[_index];
    _index++;
  }
  if (!std::regex_match(len, std::regex("^\\d+$"))) {
    // Check if the length is a positive integer
    return BencodeResult::ERROR(BencodeParserError::InvalidString);
  }
  // Skip the ':' token
  _index++;
  // Calculate the string using the length previously calculated
  auto length_str = std::stol(len);
  std::string str{};
  for (int i = 0; i < length_str && _index < _tokens.size(); i++) {
    str += _tokens[_index];
    _index++;
  }
  if (str.size() != static_cast<unsigned int>(length_str)) {
    // Exit because the string is not the same length as the given length
    return BencodeResult::ERROR(BencodeParserError::InvalidString);
  }
  return BencodeResult::OK(std::make_unique<BencodeString>(str));
}

auto BencodeParser::decode_list() -> BencodeResult {
  // The token must be in the form ['l',...,'e']
  if (_tokens.size() - _index < 2) {
    return BencodeResult::ERROR(BencodeParserError::ListFormat);
  }
  auto ptr = std::vector<std::unique_ptr<BencodeValue>>();
  // Increment index to skip the first 'l' already checked before enter the
  // function
  _index += 1;
  while (_index < _tokens.size() && _tokens[_index] != 'e') {
    auto r = decode();
    if (!r.valid()) {
      // An error occurred while decoding the list
      return r;
    }
    ptr.push_back(std::move(*r));
  }
  // Push all items but not space for 'e'
  if (_index >= _tokens.size()) {
    return BencodeResult::ERROR(BencodeParserError::ListFormat);
  }
  // Check if the list is closed with 'e'
  if (_tokens[_index] != 'e') {
    return BencodeResult::ERROR(BencodeParserError::ListFormat);
  }
  // Increment index to skip the last 'e'
  _index += 1;
  return BencodeResult::OK(std::make_unique<BencodeList>(std::move(ptr)));
}

auto BencodeParser::decode_dict() -> BencodeResult {
  // The token must be in the form ['d',...,'e']
  if (_tokens.size() - _index < 2) {
    return BencodeResult::ERROR(BencodeParserError::DictFormat);
  }
  // Increment index to skip the first 'd' already checked before enter the
  // function
  _index += 1;
  auto ptr = std::map<std::string, std::unique_ptr<BencodeValue>>();
  std::vector<std::string> keys = std::vector<std::string>();
  while (_index < _tokens.size() && _tokens[_index] != 'e') {
    auto r_key = decode();
    if (!r_key.valid()) {
      // An error occurred while decoding a dictionary key
      return r_key;
    }
    auto key = std::move(*r_key);
    // The key must be a string
    if (key->get_type() != BencodeType::String) {
      return BencodeResult::ERROR(BencodeParserError::DictKey);
    }
    // Cast the key to a BencodeString
    auto r_value = BencodeParser::decode();
    if (!r_value.valid()) {
      // An error occurred while decoding the value of a key
      return r_value;
    }
    auto key_str = dynamic_cast<BencodeString&>(*key).value();
    keys.push_back(key_str);
    ptr.insert({key_str, std::move(*r_value)});
  }
  // Push all items but not space for 'e'
  if (_index >= _tokens.size()) {
    return BencodeResult::ERROR(BencodeParserError::DictFormat);
  }
  // Check if the list is closed with 'e'
  if (_tokens[_index] != 'e') {
    return BencodeResult::ERROR(BencodeParserError::DictFormat);
  }
  // Check if the keys array is sorted by lexicographical order
  for (unsigned long i = 0; i < keys.size() - 1; i++) {
    if (keys[i] > keys[i + 1]) {
      return BencodeResult::ERROR(BencodeParserError::DictKeyOrder);
    }
  }
  // increment index to skip the last 'e'
  _index += 1;
  return BencodeResult::OK(std::make_unique<BencodeDict>(std::move(ptr)));
}

}  // namespace fur::bencode
