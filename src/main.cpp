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
  furrent.add_torrent("../extra/debian-11.4.0-amd64-netinst.iso.torrent");

  std::this_thread::sleep_for(std::chrono::seconds(30));
  return 0;
}
