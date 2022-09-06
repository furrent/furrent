#include <fstream>
#include <log/logger.hpp>
#include <sstream>

#include <furrent2.hpp>

const char* TORRENT_1 = "../extra/debian-11.4.0-amd64-netinst.iso.torrent";
const char* TORRENT_2 = "../extra/ubuntu-22.04.1-desktop-amd64.iso.torrent";

int main() {

  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  
  // TODO enable only in debug mode
  logger->set_level(spdlog::level::debug);

  fur::Furrent2 furrent;
  furrent.add_torrent(TORRENT_1);
  furrent.add_torrent(TORRENT_2);

  std::this_thread::sleep_for(std::chrono::seconds(5));
  return 0;
}
