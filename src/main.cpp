#include <iostream>
#include "bencode_parser.hpp"
#include "bencode_value.hpp"
#include <vector>
using namespace fur::bencode;
int main() {
  std::cout << "Hello, World!" << std::endl;
  BencodeParser parser;
  auto l2 = parser.decode("4:spam");
  std::cout << l2->to_string() << std::endl;
  return 0;
}
