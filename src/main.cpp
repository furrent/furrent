#include <iostream>
#include "bencode_parser.hpp"
#include "bencode_value.hpp"
using namespace fur::bencode;
int main() {
  std::cout << "Hello, World!" << std::endl;
  BencodeParser parser{};
  auto l2 = parser.decode("d3:fooi1e4:bar4:spame");
  // cast l2 in BencodeList
  std::cout << l2->to_string() << std::endl;
  return 0;
}
