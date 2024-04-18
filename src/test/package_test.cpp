#include "package.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Parsing valid version string") {
    SECTION("Valid version string") {
        std::string versionStr = "1.2.3";
        Version expectedVersion{1, 2, 3};
        REQUIRE(parseVersionString(versionStr) == expectedVersion);
    }

    SECTION("Version string with zero values") {
        std::string versionStr = "0.0.0";
        Version expectedVersion{0, 0, 0};
        REQUIRE(parseVersionString(versionStr) == expectedVersion);
    }

    SECTION("Version string with large values") {
        std::string versionStr = "123456789.987654321.1234567890";
        Version expectedVersion{123456789, 987654321, 1234567890};
        REQUIRE(parseVersionString(versionStr) == expectedVersion);
    }
}

TEST_CASE("Parsing invalid version string") {
    SECTION("Invalid format: Missing minor version") {
        std::string versionStr = "1..3";
        REQUIRE_THROWS_AS(parseVersionString(versionStr), std::invalid_argument);
    }

    SECTION("Invalid format: Extra dot") {
        std::string versionStr = "1.2.3.";
        REQUIRE_THROWS_AS(parseVersionString(versionStr), std::invalid_argument);
    }

    SECTION("Invalid format: Non-numeric character") {
        std::string versionStr = "1.2.x";
        REQUIRE_THROWS_AS(parseVersionString(versionStr), std::invalid_argument);
    }

    SECTION("Invalid format: Empty string") {
        std::string versionStr = "";
        REQUIRE_THROWS_AS(parseVersionString(versionStr), std::invalid_argument);
    }
}


TEST_CASE("Comparison operators for Version struct") {
    Version v1{1, 2, 3};
    Version v2{1, 2, 3};
    Version v3{1, 2, 4};

    SECTION("Equality (==)") {
        REQUIRE(v1 == v2);
        REQUIRE_FALSE(v1 == v3);
    }

    SECTION("Inequality (!=)") {
        REQUIRE_FALSE(v1 != v2);
        REQUIRE(v1 != v3);
    }

    SECTION("Less than (<)") {
        REQUIRE_FALSE(v1 < v2);
        REQUIRE(v1 < v3);
        REQUIRE_FALSE(v3 < v1);
    }

    SECTION("Less than or equal to (<=)") {
        REQUIRE(v1 <= v2);
        REQUIRE(v1 <= v3);
        REQUIRE_FALSE(v3 <= v1);
    }

    SECTION("Greater than (>)") {
        REQUIRE_FALSE(v1 > v2);
        REQUIRE_FALSE(v1 > v3);
        REQUIRE(v3 > v1);
    }

    SECTION("Greater than or equal to (>=)") {
        REQUIRE(v1 >= v2);
        REQUIRE_FALSE(v1 >= v3);
        REQUIRE(v3 >= v1);
    }
}
