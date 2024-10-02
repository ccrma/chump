#include "util.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("parsePackageName") {
  SECTION("valid cases") {
    auto [name, ver] = parsePackageName("Test");

    CHECK(name == "Test");
    CHECK_FALSE(ver);

    std::tie(name, ver) = parsePackageName("Test=0.9.1");
    CHECK(name == "Test");
    REQUIRE(ver);
    CHECK(ver == "0.9.1");
  }

  SECTION("invalid cases") {
    auto [name, ver] = parsePackageName("Test0.9.1");

    CHECK(name == "Test0.9.1");
    CHECK_FALSE(ver);
  }
}
