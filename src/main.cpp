#include <iostream>
#include "bencode_parser.hpp"
#include "bencode_value.hpp"
#include <vector>
using namespace fur::bencode;
int main() {
  std::cout << "Hello, World!" << std::endl;
  BencodeParser parser{};
  auto l2 = parser.decode("li1ei2ee");
  // cast l2 in BencodeList
  auto l = dynamic_cast<BencodeList*>(l2.get());
  return 0;
}
