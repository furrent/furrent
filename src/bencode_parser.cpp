//
// Created by nicof on 01/08/22.
//
#include "bencode_parser.hpp"

#include <regex>
#include "iostream"

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
      std::cout << iter->str() << std::endl;
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
  // Check if is an integer
  if(tokens.front() == "i" && tokens.back() == "e"){
    return BencodeParser::decode_int(tokens);
  }
  // Check if is a string using a regex that find "number:"
  std::regex reg_integer("^\\d+:$");
  if(std::regex_match(tokens.front(), reg_integer)) {
    return BencodeParser::decode_string(tokens);
  }

  // Check if is a list
  //if(tokens.front() == "l" && tokens.back() == "e"){
  //  // Decode all the list removing the first and last element
  //}

  throw std::invalid_argument("BencodeParser::decode(std::string decoded): invalid decoded string");

}

std::unique_ptr<BencodeValue> BencodeParser::decode_int(
    const std::vector<std::string>& tokens) {
  // The token must be in the form ["i", "number", "e"]
  if(tokens.size() < 3) {
    throw std::invalid_argument("BencodeInt::BencodeInt(std::string encoded): invalid encoded string");
  }
  if(tokens.front() != "i" || tokens.back() != "e") {
    throw std::invalid_argument("BencodeInt::BencodeInt(std::string encoded): invalid encoded string");
  }
  if(tokens[1] == "-0"){
    throw std::invalid_argument("BencodeInt::BencodeInt(std::string encoded): invalid encoded string");
  }
  return std::make_unique<BencodeInt>(std::stoi(tokens[1]));
}
std::unique_ptr<BencodeValue> BencodeParser::decode_string(
    const std::vector<std::string>& tokens) {
  // The token must be in the form ["length:", "string"]
  if(tokens.size() < 2) {
    throw std::invalid_argument("BencodeString::BencodeString(std::string encoded): invalid encoded string");
  }
  return std::make_unique<BencodeString>(tokens[1]);
}

