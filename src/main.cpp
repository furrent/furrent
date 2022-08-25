#include <iostream>

#include <log/logger.hpp>

int main() {

  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  logger->info("Hello, {}!", "World");

  return 0;
}
