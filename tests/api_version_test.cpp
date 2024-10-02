#include "chuck_version.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("ApiVersion constructor with default command", "[ApiVersion]") {
    SECTION("Invalid command") {
        // Provide a command that does not exist, the constructor should throw an exception
        REQUIRE_THROWS_AS(ApiVersion("Invalid version string"), std::runtime_error);
        REQUIRE_THROWS_AS(ApiVersion("10.1-dev"), std::runtime_error);
    }
}

TEST_CASE("ApiVersion constructor with version string", "[ApiVersion]") {
    SECTION("Valid version string") {
        REQUIRE_NOTHROW(ApiVersion("10.1"));
    }

    SECTION("Invalid version string") {
        // Provide an invalid version string, the constructor should throw an exception
        REQUIRE_THROWS_AS(ApiVersion("Invalid version string"), std::runtime_error);
    }
}

TEST_CASE("ApiVersion version numbers are correctly parsed", "[ApiVersion]") {
    SECTION("Constructor with version string") {
        ApiVersion chuck("10.1");
        REQUIRE(chuck.major == 10);
        REQUIRE(chuck.minor == 1);
    }
}

// Unit tests
TEST_CASE("ApiVersion comparison operators", "[ApiVersion]") {
    ApiVersion chuck1("10.1");
    ApiVersion chuck2("10.2");
    ApiVersion chuck3("11.0");

    SECTION("Equality") {
        REQUIRE(chuck1 == chuck1);
        REQUIRE_FALSE(chuck1 == chuck2);
        REQUIRE_FALSE(chuck1 == chuck3);
    }

    SECTION("Inequality") {
        REQUIRE_FALSE(chuck1 != chuck1);
        REQUIRE(chuck1 != chuck2);
        REQUIRE(chuck1 != chuck3);
    }

    SECTION("Less than") {
        REQUIRE(chuck1 < chuck2);
        REQUIRE(chuck1 < chuck3);
        REQUIRE_FALSE(chuck2 < chuck1);
        REQUIRE_FALSE(chuck3 < chuck1);
    }

    SECTION("Less than or equal to") {
        REQUIRE(chuck1 <= chuck1);
        REQUIRE(chuck1 <= chuck2);
        REQUIRE(chuck1 <= chuck3);
        REQUIRE_FALSE(chuck2 <= chuck1);
        REQUIRE_FALSE(chuck3 <= chuck1);
    }

    SECTION("Greater than") {
        REQUIRE(chuck2 > chuck1);
        REQUIRE(chuck3 > chuck1);
        REQUIRE_FALSE(chuck1 > chuck2);
        REQUIRE_FALSE(chuck1 > chuck3);
    }

    SECTION("Greater than or equal to") {
        REQUIRE(chuck1 >= chuck1);
        REQUIRE(chuck2 >= chuck1);
        REQUIRE(chuck3 >= chuck1);
        REQUIRE_FALSE(chuck1 >= chuck2);
        REQUIRE_FALSE(chuck1 >= chuck3);
    }
}
