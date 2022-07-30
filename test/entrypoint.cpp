#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

// This test merely checks that the testing infrastructure is working as
// intended
TEST_CASE("Trivial") { REQUIRE(1 == 1); }
