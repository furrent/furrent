#include <fstream>
#include <furrent.hpp>
#include <log/logger.hpp>

int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  logger->info("Hello, world!");

  fur::Furrent furrent;
  furrent.add_torrent("../extra/extra/debian-11.4.0-amd64-netinst.iso.torrent");
  //furrent.add_torrent("../extra/multi-file.torrent");

  //std::this_thread::sleep_for(std::chrono::seconds(30));
  while(true);
  return 0;
}
