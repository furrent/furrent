#include "bencode_parser.hpp"

#include <regex>
#include "iostream"
#include "memory"

using namespace fur::bencode;

std::string BencodeParser::encode(const BencodeValue& value) {
  return "";
}

std::vector<std::string> BencodeParser::tokenizer(const std::string& encoded) {
  std::vector<std::string> tokens;
  std::regex regexp("([idel])|(\\d+):|(-?\\d+)");
  std::sregex_token_iterator iter(encoded.begin(),
                                  encoded.end(),
                                  regexp,
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
  if(tokens.empty()){
    throw std::invalid_argument("BencodeParser::decode(std::string decoded): invalid decoded string");
  }
  _index = 0; // Reset the index
  return decode(tokens);
}

// Given a vector of tokens, returns a BencodeValue object
std::unique_ptr<BencodeValue> BencodeParser::decode(
    const std::vector<std::string>& tokens) {
  std::regex reg_string("^\\d+:$");
  std::string token;
  while(_index<tokens.size()){
    token = tokens[_index];
    if(token == "i"){
      return BencodeParser::decode_int(tokens);
    }
    if(std::regex_match(token, reg_string)) {

      return BencodeParser::decode_string(tokens);
    }
    if(token == "l"){
      // Decode all the list removing the first and last element
      return BencodeParser::decode_list(tokens);
    }
    if(token == "d"){
      return nullptr;
    }
  }
  throw std::invalid_argument("BencodeParser::decode(std::string decoded): invalid decoded string");
}

std::unique_ptr<BencodeValue> BencodeParser::decode_int(
    const std::vector<std::string>& tokens) {
  // The token must be in the form ["i", "number", "e"]
  if(tokens.size()-_index < 2) {
    throw std::invalid_argument("BencodeInt::BencodeInt(std::vector<std::string>& encoded): no tokens to decode");
  }
  if(tokens[_index] != "i" || tokens[_index+2] != "e") {
    throw std::invalid_argument("BencodeInt::BencodeInt(std::vector<std::string>& encoded): invalid encoded string");
  }
  if(tokens[_index+1] == "-0"){
    throw std::invalid_argument("BencodeInt::BencodeInt(std::vector<std::string>& encoded): invalid integer");
  }
  //"number" and "i" for the next decode
  _index+=2;
  return std::make_unique<BencodeInt>(std::stoi(tokens[_index-1]));
}
std::unique_ptr<BencodeValue> BencodeParser::decode_string(
    const std::vector<std::string>& tokens) {
  // The token must be in the form ["length:", "string"]
  if(tokens.size()-_index < 1) {
    throw std::invalid_argument("BencodeString::BencodeString(std::vector<std::string>& encoded): invalid encoded string");
  }
  _index+=2;
  return std::make_unique<BencodeString>(tokens[1]);
}
std::unique_ptr<BencodeValue> BencodeParser::decode_list(
    const std::vector<std::string>& tokens) {
  // The token must be in the form ["l",...,"e"]
  if(tokens.size()-_index < 3) {
    throw std::invalid_argument("BencodeList::BencodeList(std::vector<std::string>& encoded): invalid encoded string");
  }
  auto ptr = std::vector<std::unique_ptr<BencodeValue>>();
  // increment index to skip the first "l"
  _index+=1;
  while(tokens[_index] != "e") {
    ptr.push_back(BencodeParser::decode(tokens));
    _index+=1;
  }
  // increment index to skip the last "e"
  _index+=1;
  return std::make_unique<BencodeList>(std::move(ptr));
}
