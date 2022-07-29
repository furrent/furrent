#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace bencode {

enum class BencodeType { Integer, String, List, Dict };

class BencodeValue {
 public:
  virtual std::string to_string() const = 0;
  virtual BencodeType get_type() const = 0;
};

class BencodeInt : public BencodeValue {
 private:
  int val;

 public:
  explicit BencodeInt(int data);
  virtual std::string to_string() const override;
  virtual BencodeType get_type() const override;
  [[nodiscard]] int value() const;
};

class BencodeString : public BencodeValue {
 private:
  std::string val;

 public:
  explicit BencodeString(std::string data);
  virtual std::string to_string() const override;
  virtual BencodeType get_type() const override;
  [[nodiscard]] std::string value() const;
};

class BencodeList : public BencodeValue {
 private:
  std::vector<std::unique_ptr<BencodeValue>> list;

 public:
  explicit BencodeList(std::vector<std::unique_ptr<BencodeValue>> data);
  virtual std::string to_string() const override;
  virtual BencodeType get_type() const override;
  [[nodiscard]] std::vector<std::unique_ptr<BencodeValue>> value() const;
};

class BencodeDict : public BencodeValue {
 private:
  std::map<std::string, std::unique_ptr<BencodeValue>> dict;

 public:
  explicit BencodeDict(
      std::map<std::string, std::unique_ptr<BencodeValue>> data);
  virtual std::string to_string() const override;
  virtual BencodeType get_type() const override;
  [[nodiscard]] std::map<std::string, std::unique_ptr<BencodeValue>> value()
      const;
};
}  // namespace bencode