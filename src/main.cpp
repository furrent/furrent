#include <iostream>

#include "furrent.hpp"

int main() {
  fur::Furrent f{};
  f.add_torrent("../extra/debian-11.4.0-amd64-netinst.iso.torrent");
  f.print_status();
  //auto t = f.pick_torrent();
  //if(t.has_tasks()){
  //  auto task = t.pick_task();
  //  std::cout << "Task: " << task.index << std::endl;
  //  std::cout << t.has_tasks() << std::endl;
  //}



  return 0;
}
