#include "util.h"

#include <catch2/catch_test_macros.hpp>

namespace fs = std::filesystem;

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

TEST_CASE("is_subpath", "[path]") {
    REQUIRE( is_subpath("a/b/c", "a/b") );
    REQUIRE_FALSE( is_subpath("a/b/c", "b") );
    REQUIRE_FALSE( is_subpath("a", "a/b/c") );

    REQUIRE( is_subpath("a/b/c/..", "a/") );
    REQUIRE_FALSE( is_subpath("./", "./../oops") );

    REQUIRE( is_subpath("./poop", "./") );
    REQUIRE_FALSE( is_subpath("./..", "./") );
    REQUIRE_FALSE( is_subpath("./../../../..", "./") );

}
