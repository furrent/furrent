#include "bencode_parser.hpp"

#include <regex>

using namespace fur::bencode;

std::string BencodeParser::encode(const BencodeValue& value) {
  return value.to_string();
}

std::unique_ptr<BencodeValue> BencodeParser::decode(
    const std::string& decoded) {
  _tokens = decoded;
  _index = 0;  // Reset the index
  auto r = decode();
  if(_index != _tokens.size()) {
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
    if (token == 'i') {
      return BencodeParser::decode_int();
    } else if (token<= '9' && token >= '0') {
      return BencodeParser::decode_string();
    } else if (token == 'l') {
      return BencodeParser::decode_list();
    } else if (token == 'd') {
      return BencodeParser::decode_dict();
    }
  }
  throw std::invalid_argument(
      "BencodeParser::decode(std::string decoded): invalid string");
}

std::unique_ptr<BencodeValue> BencodeParser::decode_int() {
  // The token must be in the form ['i', 'number', 'e']
  if (_tokens.size() - _index < 3) {
    throw std::invalid_argument(
        "BencodeParser::decode_int(): no tokens to decode");
  }
  // Skip the 'i' token already checked before entering this function
  _index++;
  // Decoding the integer
  std::string integer{};
  while(_index < _tokens.size() && _tokens[_index] != 'e') {
    integer += _tokens[_index];
    _index++;
  }
  if (_index == _tokens.size()) {
    // No space for the 'e' token
    throw std::invalid_argument(
        "BencodeParser::decode_int(): invalid encoded string");
  }else if(_tokens[_index]!='e'){
    // Missing 'e' at the end of the integer
    throw std::invalid_argument(
        "BencodeParser::decode_int(): invalid encoded string");
  }else if (integer == "-0") {
    // The "-0" is not a valid integer
    throw std::invalid_argument(
        "BencodeParser::decode_int(): negative zero is not allowed");
  }else if (!std::regex_match(integer, std::regex ("^-?\\d+$"))){
    // Check if the integer is a string of digits with sign
    throw std::invalid_argument(
        "BencodeParser::decode_int(): invalid integer");
  }
  // Skip 'e' at the end
  _index++;
  return std::make_unique<BencodeInt>(std::stoi(integer));
}
std::unique_ptr<BencodeValue> BencodeParser::decode_string() {
  // The token must be in the form ['length', ':', 'string']
  if (_tokens.size() - _index < 3) {
    throw std::invalid_argument(
        "BencodeParser::bencode_string(): invalid encoded string");
  }
  // Calculate the string length until the ':' token
  std::string len{};
  while(_index < _tokens.size() && _tokens[_index] != ':') {
    len += _tokens[_index];
    _index++;
  }
  if(len=="-0"){
    // The "-0" is not a valid integer
    throw std::invalid_argument(
        "BencodeParser::decode_string(): negative zero is not allowed");
  }else if(!std::regex_match(len, std::regex("^\\d+$"))) {
    // Check if the length is a positive integer
    throw std::invalid_argument(
        "BencodeParser::decode_string(): invalid encoded string length");
  }
  // Skip the ':' token
  _index++;
  // Calculate the string using the length previously calculated
  auto length_str = std::stoi(len);
  std::string str{};
  for(int i = 0; i<length_str && _index < _tokens.size(); i++){
    str += _tokens[_index];
    _index++;
  }
  if(str.size() != static_cast<unsigned int>(length_str)) {
    // Exit because the string is not the same length as the given length
    throw std::invalid_argument(
        "BencodeParser::decode_string(): invalid encoded string length");
  }
  return std::make_unique<BencodeString>(str);
}
std::unique_ptr<BencodeValue> BencodeParser::decode_list() {
  // The token must be in the form ['l',...,'e']
  if (_tokens.size() - _index < 2) {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(): invalid encoded string");
  }
  auto ptr = std::vector<std::unique_ptr<BencodeValue>>();
  // Increment index to skip the first 'l' already checked before enter the
  // function
  _index += 1;
  while (_index < _tokens.size() && _tokens[_index] != 'e') {
    ptr.push_back(BencodeParser::decode());
  }
  // Push all items but not space for 'e'
  if (_index >= _tokens.size()) {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(): invalid encoded string");
  }
  // Check if the list is closed with 'e'
  if (_tokens[_index] != 'e') {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(): missing end of the list");
  }
  // Increment index to skip the last 'e'
  _index += 1;
  return std::make_unique<BencodeList>(std::move(ptr));
}
std::unique_ptr<BencodeValue> BencodeParser::decode_dict() {
  // The token must be in the form ['d',...,'e']
  if (_tokens.size() - _index < 2) {
    throw std::invalid_argument(
        "BencodeParser::bencode_dict(): invalid encoded string");
  }
  // Increment index to skip the first 'd' already checked before enter the
  // function
  _index += 1;
  auto ptr = std::map<std::string, std::unique_ptr<BencodeValue>>();
  std::vector<std::string> keys = std::vector<std::string>();
  while (_index < _tokens.size() && _tokens[_index] != 'e') {
    auto key = std::move(BencodeParser::decode());
    // The key must be a string
    if (key->get_type() != BencodeType::String) {
      throw std::invalid_argument(
          "BencodeParser::bencode_dict(): invalid encoded string of a key");
    }
    // Cast the key to a BencodeString
    auto value = BencodeParser::decode();
    auto key_str = dynamic_cast<BencodeString&>(*key).value();
    keys.push_back(key_str);
    ptr.insert({key_str, std::move(value)});
  }
  // Push all items but not space for 'e'
  if (_index >= _tokens.size()) {
    throw std::invalid_argument(
        "BencodeParser::bencode_list(): invalid encoded string");
  }
  // Check if the list is closed with 'e'
  if (_tokens[_index] != 'e') {
    throw std::invalid_argument(
        "BencodeParser::bencode_dict(): missing end of the dictionary");
  }
  // Check if the keys array is sorted by lexicographical order
  for (unsigned long i = 0; i < keys.size() - 1; i++) {
    if (keys[i] > keys[i + 1]) {
      throw std::invalid_argument(
          "BencodeParser::bencode_dict(): keys of dict are not sorted");
    }
  }
  // increment index to skip the last 'e'
  _index += 1;
  return std::make_unique<BencodeDict>(std::move(ptr));
}
