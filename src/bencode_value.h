//
// Created by nicof on 28/07/22.
//

#ifndef FURRENT_BENCODE_VALUE_H
#define FURRENT_BENCODE_VALUE_H

#include <string>
#include <map>
#include <vector>

namespace bencode {
  class BencodeValue {
    public:
      BencodeValue() = default;
      ~BencodeValue() = default;
      virtual std::string toString() = 0;
  };

  class BencodeInt : public BencodeValue {
    private:
      int val;
    public:
      explicit BencodeInt(std::string encoded);
      ~BencodeInt() = default;
      std::string toString() override;
      [[nodiscard]] int value() const;
  };

  class BencodeString : public BencodeValue {
   private:
    std::string val;
   public:
    explicit BencodeString(const std::string& encoded);
    ~BencodeString() = default;
    std::string toString() override;
    [[nodiscard]] std::string value();
  };

  class BencodeList : public BencodeValue {
   private:
    std::vector<BencodeValue*> list;
   public:
    explicit BencodeList(std::string encoded);
    ~BencodeList() = default;
    std::string toString() override;
    std::vector <BencodeValue*> value();
  };

  class BencodeDict : public BencodeValue {
   private:
    std::map<std::string, BencodeValue*> dict;
   public:
    explicit BencodeDict(std::string encoded);
    ~BencodeDict() = default;
    std::string toString() override;
    std::map <std::string, BencodeValue*> value();
  };
}

#endif  // FURRENT_BENCODE_VALUE_H
