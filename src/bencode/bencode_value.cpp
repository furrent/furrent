#include "bencode_value.hpp"

#include <regex>
#include <string>
#include <utility>

using namespace fur::bencode;

// ================
// BencodeInt
// ================
BencodeInt::BencodeInt(int64_t data) { _val = data; }

std::string BencodeInt::to_string() const {
  // Return the encoded string of the BencodeInt i + val + e
  return "i" + std::to_string(_val) + "e";
}

BencodeType BencodeInt::get_type() const { return BencodeType::Integer; }

int64_t BencodeInt::value() const { return _val; }

// ================
// BencodeString
// ================
BencodeString::BencodeString(std::string data) { _val = std::move(data); }

std::string BencodeString::to_string() const {
  // Return the encoded string of the BencodeString size: + val
  return std::to_string(_val.size()) + ":" + _val;
}

BencodeType BencodeString::get_type() const { return BencodeType::String; }

std::string& BencodeString::value() { return _val; }

// ================
// BencodeList
// ================
BencodeList::BencodeList(std::vector<std::unique_ptr<BencodeValue>> data) {
  _val = std::move(data);
}

std::string BencodeList::to_string() const {
  // Return the encoded string of the BencodeList l + ... + e
  std::string ret = "l";
  for (auto& v : _val) {
    ret += v->to_string();
  }
  ret += "e";
  return ret;
}

BencodeType BencodeList::get_type() const { return BencodeType::List; }

std::vector<std::unique_ptr<BencodeValue>>& BencodeList::value() {
  return _val;
}

// ================
// BencodeList
// ================
BencodeDict::BencodeDict(
    std::map<std::string, std::unique_ptr<BencodeValue>> data) {
  _val = std::move(data);
}
std::string BencodeDict::to_string() const {
  // Return the encoded string of the BencodeDict d + ... + e
  std::string ret = "d";
  // Map is already sorted by key, so we can just iterate over it
  for (auto const& [key, val] : _val) {
    // Format of element is size:key + value
    ret += std::to_string(key.size()) + ":" + key;
    ret += val->to_string();
  }
  ret += "e";
  return ret;
}

BencodeType BencodeDict::get_type() const { return BencodeType::Dict; }

std::map<std::string, std::unique_ptr<BencodeValue>>& BencodeDict::value() {
  return _val;
}
