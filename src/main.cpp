#include <iostream>

#include "furrent.hpp"

int main() {
  std::cout << "Hello, World!" << std::endl;
  fur::Furrent f{};
  f.add_torrent("../extra/debian-11.4.0-amd64-netinst.iso.torrent");
  auto t = f.pick_torrent();
  f.print_status();
  t.pick_task();
  f.print_status();
  return 0;
}
