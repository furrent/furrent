#include <iostream>

#include "furrent.hpp"

int main() {
  std::cout << "Hello, World!" << std::endl;
  fur::Furrent f{};
  f.add_torrent("/home/nicof/Desktop/UNIVR/furrent/extra/debian-11.4.0-amd64-netinst.iso.torrent");
  f.pick_torrent();
  f.print_status();
  return 0;
}
