
#include <cstdint>
#include <fstream>
#include <iostream>
#include <log/logger.hpp>
#include <sstream>

#include <furrent.hpp>

int main(int argc, char* argv[]) {

  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");

  fur::Furrent furrent;
  //furrent.add_torrent("../extra/raspios-2022-09-06-raspios-bullseye-armhf.img.xz.torrent");
  furrent.add_torrent("../extra/debian-11.5.0-amd64-i386-netinst.iso.torrent");

  //std::this_thread::sleep_for(std::chrono::seconds(30));
  while(true);
  return 0;
}
