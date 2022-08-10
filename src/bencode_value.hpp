#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

/// Contains the structure for decoding and encoding bencode data
namespace fur::bencode{

enum class BencodeType { Integer, String, List, Dict };

class BencodeValue {
 public:
  [[nodiscard]] virtual std::string to_string() const = 0;
  /// Returns the type of the bencode value as a BencodeType enum
  [[nodiscard]] virtual BencodeType get_type() const = 0;
};

class BencodeInt : public BencodeValue {
 private:
  int _val;

 public:
  explicit BencodeInt(int data);
  [[nodiscard]] std::string to_string() const override;
  [[nodiscard]] BencodeType get_type() const override;
  /// Returns the integer value of the bencode value
  [[nodiscard]] int value() const;
};

class BencodeString : public BencodeValue {
 private:
  std::string _val;

 public:
  explicit BencodeString(std::string data);
  [[nodiscard]] std::string to_string() const override;
  [[nodiscard]] BencodeType get_type() const override;
  /// Returns the string value of the bencode value
  [[nodiscard]] std::string& value();
};

class BencodeList : public BencodeValue {
 private:
  std::vector<std::unique_ptr<BencodeValue>> _val;

 public:
  explicit BencodeList(std::vector<std::unique_ptr<BencodeValue>> data);
  [[nodiscard]] std::string to_string() const override;
  [[nodiscard]] BencodeType get_type() const override;
  /// Returns the list of BencodeValue objects that are contained in the list
  [[nodiscard]] std::vector<std::unique_ptr<BencodeValue>>&
  value();
};

class BencodeDict : public BencodeValue {
 private:
  std::map<std::string, std::unique_ptr<BencodeValue>> _val;

 public:
  explicit BencodeDict(
      std::map<std::string, std::unique_ptr<BencodeValue>> data);
  [[nodiscard]] std::string to_string() const override;
  [[nodiscard]] BencodeType get_type() const override;
  /// Returns the dictionary of BencodeValue objects that are contained in the
  /// dict
  [[nodiscard]] std::map<std::string, std::unique_ptr<BencodeValue>>&
  value();
};

}