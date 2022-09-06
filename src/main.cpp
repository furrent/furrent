#include <fstream>
#include <log/logger.hpp>
#include <sstream>

#include "bencode/bencode_parser.hpp"
#include "torrent.hpp"

int main() {

  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  logger->info("Hello, world!");

  return 0;
}
