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
  _index = 0;  // Reset the index
  return decode(tokens);
}

// Given a vector of tokens, returns a BencodeValue object
std::unique_ptr<BencodeValue> BencodeParser::decode(
    const std::vector<std::string>& tokens) {
  std::regex reg_string("^\\d+:$");
  std::string token;
  if (!tokens.empty()) {
    token = tokens[_index];
    if (token == "i") {
      // If the first token is "i", then it's an integer, so we have a fixed
      // length
      if (_index == 0 && tokens.size() != 3) {
        throw std::invalid_argument(
            "BencodeParser::decode(std::string decoded): invalid string");
      }
      return BencodeParser::decode_int(tokens);
    } else if (std::regex_match(token, reg_string)) {
      // If the first tokes is for a string, then it's a string, so we have a
      // fixed length
      if (_index == 0 && tokens.size() != 2) {
        throw std::invalid_argument(
            "BencodeParser::decode(std::string decoded): invalid string");
      }
      return BencodeParser::decode_string(tokens);
    } else if (token == "l") {
      return BencodeParser::decode_list(tokens);
    } else if (token == "d") {
      return BencodeParser::decode_dict(tokens);
    }
    throw std::invalid_argument(
        "BencodeParser::decode(std::string decoded): invalid string");
  }
}

std::unique_ptr<BencodeValue> BencodeParser::decode_int(
    const std::vector<std::string>& tokens) {
  // The token must be in the form ["i", "number", "e"]
  if (tokens.size() - _index < 3) {
    throw std::invalid_argument(
        "BencodeParser::decode_int(std::vector<std::string>& encoded): no "
        "tokens to decode");
  }
  if (tokens[_index] != "i" || tokens[_index + 2] != "e") {
    throw std::invalid_argument(
        "BencodeParser::decode_int(std::vector<std::string>& encoded): invalid "
        "encoded string");
  }
  if (tokens[_index + 1] == "-0") {
    throw std::invalid_argument(
        "BencodeParser::decode_int(std::vector<std::string>& encoded): "
        "negative zero is not allowed");
  }
  // skip "e", "number" and "i" for the next decode
  _index += 3;
  return std::make_unique<BencodeInt>(std::stoi(tokens[_index - 2]));
}
std::unique_ptr<BencodeValue> BencodeParser::decode_string(
    const std::vector<std::string>& tokens) {
  // The token must be in the form ["length:", "string"]
  if (tokens.size() - _index < 2) {
    throw std::invalid_argument(
        "BencodeParser::bencode_string(std::vector<std::string>& encoded): "
        "invalid encoded string");
  }
  std::string str = tokens[_index + 1];
  // Transform n: to a integer
  std::string length = tokens[_index].substr(0, tokens[_index].size() - 1);
  int len = std::stoi(length);
  // Check if the length of the string is correct
  if (len < 0 || (unsigned int)len != str.size()) {
    throw std::invalid_argument(
        "BencodeParser::bencode_string(std::vector<std::string>& encoded): "
        "invalid string length");
  }
  // "length:" and "string" for the next decode
  _index += 2;
  return std::make_unique<BencodeString>(str);
}
std::unique_ptr<BencodeValue> BencodeParser::decode_list(
    const std::vector<std::string>& tokens) {
  // The token must be in the form ["l",...,"e"]
  if (tokens.size() - _index < 4) {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(std::vector<std::string>& encoded): "
        "invalid encoded string");
  }
  auto ptr = std::vector<std::unique_ptr<BencodeValue>>();
  // Increment index to skip the first "l"
  _index += 1;
  while (_index < tokens.size() && tokens[_index] != "e") {
    ptr.push_back(BencodeParser::decode(tokens));
  }
  // Push all items but not space for "e"
  if (_index == tokens.size()) {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(std::vector<std::string>& encoded): "
        "invalid encoded string");
  }
  // Check if the list is closed with "e"
  if (tokens[_index] != "e") {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(std::vector<std::string>& encoded): "
        "missing end of the list");
  }

  // Increment index to skip the last "e"
  _index += 1;
  return std::make_unique<BencodeList>(std::move(ptr));
}
std::unique_ptr<BencodeValue> BencodeParser::decode_dict(
    const std::vector<std::string>& tokens) {
  // The token must be in the form ["d",...,"e"]
  if (tokens.size() - _index < 4) {
    throw std::invalid_argument(
        "BencodeParser::bencode_dict(std::vector<std::string>& encoded): "
        "invalid encoded string");
  }
  // increment index to skip the first "d"
  _index += 1;
  auto ptr = std::map<std::string, std::unique_ptr<BencodeValue>>();
  std::vector<std::string> keys = std::vector<std::string>();
  while (_index < tokens.size() && tokens[_index] != "e") {
    auto key = std::move(BencodeParser::decode(tokens));
    // The key must be a string
    if (key->get_type() != BencodeType::String) {
      throw std::invalid_argument(
          "BencodeParser::bencode_dict(std::vector<std::string>& encoded): "
          "invalid encoded string of a key");
    }
    // Cast the key to a BencodeString
    auto value = BencodeParser::decode(tokens);
    auto key_str = dynamic_cast<BencodeString&>(*key).value();
    keys.push_back(key_str);
    ptr.insert({key_str, std::move(value)});
  }
  // Push all items but not space for "e"
  if (_index == tokens.size()) {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(std::vector<std::string>& encoded): "
        "invalid encoded stringgggg");
  }
  // Check if the list is closed with "e"
  if (_index < tokens.size() && tokens[_index] != "e") {
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
