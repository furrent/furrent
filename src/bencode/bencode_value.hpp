#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

/// Contains the structure for decoding and encoding bencode data
namespace fur::bencode {

/// Enumeration for the different types of bencode data
enum class BencodeType { Integer, String, List, Dict };

class BencodeValue {
 public:
  /// Returns the string representation of the bencode value
  [[nodiscard]] virtual std::string to_string() const = 0;
  /// Returns the type of the bencode value as a BencodeType enum
  [[nodiscard]] virtual BencodeType get_type() const = 0;
  virtual ~BencodeValue() = default;
};

class BencodeInt : public BencodeValue {
 private:
  /// The integer value of the bencode value
  long _val;

 public:
  explicit BencodeInt(long data);
  [[nodiscard]] std::string to_string() const override;
  [[nodiscard]] BencodeType get_type() const override;
  /// Returns the integer value of the bencode value
  [[nodiscard]] long value() const;
};

class BencodeString : public BencodeValue {
 private:
  /// The string value of the bencode value
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
  /// The list of bencode values of the bencode value
  std::vector<std::unique_ptr<BencodeValue>> _val;

 public:
  explicit BencodeList(std::vector<std::unique_ptr<BencodeValue>> data);
  [[nodiscard]] std::string to_string() const override;
  [[nodiscard]] BencodeType get_type() const override;
  /// Returns the list of BencodeValue objects that are contained in the list
  [[nodiscard]] std::vector<std::unique_ptr<BencodeValue>>& value();
};

class BencodeDict : public BencodeValue {
 private:
  /// The dictionary of bencode values of the bencode value
  std::map<std::string, std::unique_ptr<BencodeValue>> _val;

 public:
  /// Constructs a BencodeDict object from a map of strings and BencodeValue
  explicit BencodeDict(
      std::map<std::string, std::unique_ptr<BencodeValue>> data);
  [[nodiscard]] std::string to_string() const override;
  [[nodiscard]] BencodeType get_type() const override;
  /// Returns the dictionary of BencodeValue objects that are contained in the
  /// dict
  [[nodiscard]] std::map<std::string, std::unique_ptr<BencodeValue>>& value();
};

}  // namespace fur::bencode