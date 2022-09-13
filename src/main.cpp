#include <fstream>
#include <log/logger.hpp>
#include <sstream>

int main() {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");

  // TODO enable only in debug mode
  logger->set_level(spdlog::level::debug);
  logger->debug("This is a debug message");

  return 0;
}
