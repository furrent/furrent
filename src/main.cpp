#include <iostream>
#include <vector>

#include "tokenizer.h"

int main() {
  std::vector<std::string> tokens = {};
  bencode::Tokenizer::tokenize("d3:bar4:spam3:fooi42ee", tokens);
  std::cout << tokens.size() << std::endl;
  for (const auto& token : tokens) {
    std::cout << token << std::endl;
  }
  return 0;
}
