//
// Created by nicof on 28/07/22.
//

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

/// Contains the structure for decoding and encoding bencode data
namespace fur::bencode{

/// Represents a possible type of a bencode value
enum class BencodeType { Integer, String, List, Dict };

/// The base class for all bencode values
class BencodeValue {
 public:
  /// Computes the string of a bencode value in bencode format
  virtual std::string to_string() const = 0;
  /// Returns the type of the bencode value as a BencodeType enum
  virtual BencodeType get_type() const = 0;
};

class BencodeInt : public BencodeValue {
 private:
  int val;

 public:
  explicit BencodeInt(int data);
  virtual std::string to_string() const override;
  virtual BencodeType get_type() const override;
  /// Returns the integer value of the bencode value
  [[nodiscard]] int value();
};

class BencodeString : public BencodeValue {
 private:
  std::string val;

 public:
  explicit BencodeString(std::string data);
  virtual std::string to_string() const override;
  virtual BencodeType get_type() const override;
  /// Returns the string value of the bencode value
  [[nodiscard]] std::string& value();
};

class BencodeList : public BencodeValue {
 private:
  std::vector<std::unique_ptr<BencodeValue>> list;

 public:
  explicit BencodeList(std::vector<std::unique_ptr<BencodeValue>> data);
  virtual std::string to_string() const override;
  virtual BencodeType get_type() const override;
  /// Returns the list of BencodeValue objects that are contained in the list
  [[nodiscard]] std::unique_ptr<std::vector<std::unique_ptr<BencodeValue>>>&
  value();
};

class BencodeDict : public BencodeValue {
 private:
  std::map<std::string, std::unique_ptr<BencodeValue>> dict;

 public:
  explicit BencodeDict(
      std::map<std::string, std::unique_ptr<BencodeValue>> data);
  virtual std::string to_string() const override;
  virtual BencodeType get_type() const override;
  /// Returns the dictionary of BencodeValue objects that are contained in the
  /// dict
  [[nodiscard]] std::map<std::string, std::unique_ptr<BencodeValue>>&
  value();
};

}