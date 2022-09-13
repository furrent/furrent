#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"
#include "log/logger.hpp"

int main(int argc, char* argv[]) {
  fur::log::initialize_custom_logger();
  auto logger = spdlog::get("custom");
  logger->set_level(spdlog::level::debug);

  return Catch::Session().run(argc, argv);
}

// This test merely checks that the testing infrastructure is working as
// intended
TEST_CASE("Trivial") { REQUIRE(1 == 1); }
