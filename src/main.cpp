#include <iostream>

#include "furrent.hpp"

int main() {
  fur::Furrent f{};
  f.add_torrent("../extra/debian-11.4.0-amd64-netinst.iso.torrent");
  f.print_status();
  auto t = f.pick_torrent();
  t.pick_task();
  return 0;
}
