#include "bencode_parser.hpp"

#include <regex>

#include "memory"

using namespace fur::bencode;

std::string BencodeParser::encode(const BencodeValue& value) {
  return value.to_string();
}

std::vector<std::string> BencodeParser::tokenizer(const std::string& encoded) {
  std::vector<std::string> tokens;
  std::regex regexp("([idel])|(\\d+):|(-?\\d+)");
  std::sregex_token_iterator iter(encoded.begin(), encoded.end(), regexp,
                                  {-1, 0});
  std::sregex_token_iterator end;
  while (iter != end) {
    if (iter->length() > 0) {
      tokens.push_back(*iter);
    }
    ++iter;
  }
  return tokens;
}

std::unique_ptr<BencodeValue> BencodeParser::decode(
    const std::string& decoded) {
  std::vector<std::string> tokens = tokenizer(decoded);
  if (tokens.empty()) {
    throw std::invalid_argument(
        "BencodeParser::decode(std::string decoded): invalid decoded string");
  }
  _tokens = tokens;
  _index = 0;  // Reset the index
  auto r = decode();
  if(_index != tokens.size()) {
    throw std::invalid_argument(
        "BencodeParser::decode(std::string decoded): invalid decoded string");
  }
  return r;
}

// Given a vector of tokens, returns a BencodeValue object
std::unique_ptr<BencodeValue> BencodeParser::decode() {
  std::regex reg_string("^\\d+:$");
  if (!_tokens.empty()) {
    auto token = _tokens[_index];
    if (token == "i") {
      return BencodeParser::decode_int();
    } else if (std::regex_match(token, reg_string)) {
      return BencodeParser::decode_string();
    } else if (token == "l") {
      return BencodeParser::decode_list();
    } else if (token == "d") {
      return BencodeParser::decode_dict();
    }
  }
  throw std::invalid_argument(
      "BencodeParser::decode(std::string decoded): invalid string");
}

std::unique_ptr<BencodeValue> BencodeParser::decode_int() {
  // The token must be in the form ["i", "number", "e"]
  if (_tokens.size() - _index < 3) {
    throw std::invalid_argument(
        "BencodeParser::decode_int(std::vector<std::string>& encoded): no "
        "tokens to decode");
  }
  if (_tokens[_index] != "i" || _tokens[_index + 2] != "e") {
    throw std::invalid_argument(
        "BencodeParser::decode_int(std::vector<std::string>& encoded): invalid "
        "encoded string");
  }
  if (_tokens[_index + 1] == "-0") {
    throw std::invalid_argument(
        "BencodeParser::decode_int(std::vector<std::string>& encoded): "
        "negative zero is not allowed");
  }
  // skip "e", "number" and "i" for the next decode
  _index += 3;
  return std::make_unique<BencodeInt>(std::stoi(_tokens[_index - 2]));
}
std::unique_ptr<BencodeValue> BencodeParser::decode_string() {
  // The token must be in the form ["length:", "string"]
  if (_tokens.size() - _index < 2) {
    throw std::invalid_argument(
        "BencodeParser::bencode_string(std::vector<std::string>& encoded): "
        "invalid encoded string");
  }
  std::string str = _tokens[_index + 1];
  // Transform n: to a integer
  std::string length = _tokens[_index].substr(0, _tokens[_index].size() - 1);
  int len = std::stoi(length);
  // Check if the length of the string is correct
  if (len < 0 || static_cast<unsigned int>(len) != str.size()) {
    throw std::invalid_argument(
        "BencodeParser::bencode_string(std::vector<std::string>& encoded): "
        "invalid string length");
  }
  // "length:" and "string" for the next decode
  _index += 2;
  return std::make_unique<BencodeString>(str);
}
std::unique_ptr<BencodeValue> BencodeParser::decode_list() {
  // The token must be in the form ["l",...,"e"]
  if (_tokens.size() - _index < 2) {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(std::vector<std::string>& encoded): "
        "invalid encoded string");
  }
  auto ptr = std::vector<std::unique_ptr<BencodeValue>>();
  // Increment index to skip the first "l"
  _index += 1;
  while (_index < _tokens.size() && _tokens[_index] != "e") {
    ptr.push_back(BencodeParser::decode());
  }
  // Push all items but not space for "e"
  if (_index == _tokens.size()) {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(std::vector<std::string>& encoded): "
        "invalid encoded string");
  }
  // Check if the list is closed with "e"
  if (_tokens[_index] != "e") {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(std::vector<std::string>& encoded): "
        "missing end of the list");
  }

  // Increment index to skip the last "e"
  _index += 1;
  return std::make_unique<BencodeList>(std::move(ptr));
}
std::unique_ptr<BencodeValue> BencodeParser::decode_dict() {
  // The token must be in the form ["d",...,"e"]
  if (_tokens.size() - _index < 2) {
    throw std::invalid_argument(
        "BencodeParser::bencode_dict(std::vector<std::string>& encoded): "
        "invalid encoded string");
  }
  // increment index to skip the first "d"
  _index += 1;
  auto ptr = std::map<std::string, std::unique_ptr<BencodeValue>>();
  std::vector<std::string> keys = std::vector<std::string>();
  while (_index < _tokens.size() && _tokens[_index] != "e") {
    auto key = std::move(BencodeParser::decode());
    // The key must be a string
    if (key->get_type() != BencodeType::String) {
      throw std::invalid_argument(
          "BencodeParser::bencode_dict(std::vector<std::string>& encoded): "
          "invalid encoded string of a key");
    }
    // Cast the key to a BencodeString
    auto value = BencodeParser::decode();
    auto key_str = dynamic_cast<BencodeString&>(*key).value();
    keys.push_back(key_str);
    ptr.insert({key_str, std::move(value)});
  }
  // Push all items but not space for "e"
  if (_index == _tokens.size()) {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(std::vector<std::string>& encoded): "
        "invalid encoded string");
  }
  // Check if the list is closed with "e"
  if (_tokens[_index] != "e") {
    throw std::invalid_argument(
        "BencodeParser::bencode_dict(std::vector<std::string>& encoded): "
        "missing end of the dictionary");
  }
  // Check if the keys array is sorted by lexicographical order
  for (unsigned long i = 0; i < keys.size() - 1; i++) {
    if (keys[i] > keys[i + 1]) {
      throw std::invalid_argument(
          "BencodeParser::bencode_dict(std::vector<std::string>& encoded): "
          "keys of dict are not sorted");
    }
  }
  // increment index to skip the last "e"
  _index += 1;
  return std::make_unique<BencodeDict>(std::move(ptr));
}
