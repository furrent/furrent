#include <iostream>
#include <vector>

#include "tokenizer.h"
#include "bencode_value.h"

int main() {
  std::vector<std::string> tokens = {};
  bencode::Tokenizer::tokenize("d3:bar4:spam3:fooi42ee", tokens);
  auto c = bencode::BencodeInt("i-0e");
  std::cout << c.value() << std::endl;
  return 0;
}
