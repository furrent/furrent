//
// Created by nicof on 28/07/22.
//

#include "bencode_value.h"

#include <regex>
#include <stdexcept>

// ================
// BencodeInt
// ================
bencode::BencodeInt::BencodeInt(std::string encoded) {
  // The string encoded must start with 'i' and end with 'e'
  if(encoded.front() != 'i' || encoded.back() != 'e' || encoded.size() < 3) {
    throw std::invalid_argument("BencodeInt::BencodeInt(std::string encoded): invalid encoded string");
  }
  // Regex to check if the string is in the format i[0-9]+e
  std::regex number_regex("^i[-]?[0-9]+e$");
  if(!std::regex_match(encoded, number_regex)) {
    throw std::invalid_argument("BencodeInt::BencodeInt(std::string encoded): invalid encoded string");
  }
  // Remove the 'i' and 'e' from the string
  encoded = encoded.substr(1, encoded.size() - 2);
  if(encoded == "-0"){
    throw std::invalid_argument("BencodeInt::BencodeInt(std::string encoded): invalid encoded string");
  }
  val = std::stoi(encoded);
}
std::string bencode::BencodeInt::toString() {
  // Return the encoded string of the BencodeInt i + val + e
  return "i" + std::to_string(val) + "e";
}

int bencode::BencodeInt::value() const {
  return val;
}

// ================
// BencodeString
// ================
bencode::BencodeString::BencodeString(const std::string& encoded) {
  // TODO implement the constructor
}

std::string bencode::BencodeString::toString() {
  return "";
}

std::string bencode::BencodeString::value() {
  return this->val;
}

// ================
// BencodeList
// ================

bencode::BencodeList::BencodeList(std::string encoded) {

}
std::string bencode::BencodeList::toString() {
  return "";
}
std::vector<bencode::BencodeValue*> bencode::BencodeList::value() {
  return this->list;
}

// ================
// BencodeDict
// ================
bencode::BencodeDict::BencodeDict(std::string encoded) {

}

std::string bencode::BencodeDict::toString() {
  return "";
}
std::map<std::string, bencode::BencodeValue*> bencode::BencodeDict::value() {
  return this->dict;
}

