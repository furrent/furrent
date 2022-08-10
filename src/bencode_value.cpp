#include "bencode_value.hpp"

#include <regex>
#include <string>
#include <utility>

using namespace fur::bencode;

// ================
// BencodeInt
// ================
BencodeInt::BencodeInt(int data) {
  _val = data;
}

std::string BencodeInt::to_string() const {
  // Return the encoded string of the BencodeInt i + val + e
  return "i" + std::to_string(_val) + "e";
}

BencodeType BencodeInt::get_type() const {
  return BencodeType::Integer;
}

int BencodeInt::value() const {
  return _val;
}

// ================
// BencodeString
// ================
BencodeString::BencodeString(std::string data) {
  _val = std::move(data);
}

std::string BencodeString::to_string() const {
  return std::to_string(_val.size()) + ":" + _val;
}

BencodeType BencodeString::get_type() const {
  return BencodeType::Integer;
}

std::string& BencodeString::value() {
  return _val;
}

// ================
// BencodeList
// ================
BencodeList::BencodeList(std::vector<std::unique_ptr<BencodeValue>> data) {
  _val = std::move(data);
}

std::string BencodeList::to_string() const {
  std::string ret = "l";
  for (auto& v : _val) {
    ret += v->to_string();
  }
  ret += "e";
  return ret;
}

BencodeType BencodeList::get_type() const {
  return BencodeType::List;
}

std::vector<std::unique_ptr<BencodeValue>>& BencodeList::value() {
  return _val;
}

