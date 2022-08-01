//
// Created by nicof on 28/07/22.
//

#include "bencode_value.hpp"

#include <regex>
#include <string>

using namespace fur::bencode;

// ================
// BencodeInt
// ================
BencodeInt::BencodeInt(int data) {
  val = data;
}
std::string BencodeInt::to_string() const {
  // Return the encoded string of the BencodeInt i + val + e
  return "i" + std::to_string(val) + "e";
}
BencodeType BencodeInt::get_type() const {
  return BencodeType::Integer;
}

// ================
// BencodeString
// ================
BencodeString::BencodeString(std::string data) {
  val = data;
}

std::string BencodeString::to_string() const {
  return std::to_string(val.size()) + ":" + val;
}
BencodeType BencodeString::get_type() const {
  return BencodeType::Integer;
}
