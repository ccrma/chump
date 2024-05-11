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

TEST_CASE("latest_version returns the highest compatible version", "[latest_version]") {
    // Mockup versions
    PackageVersion v1 {"1.0.0", "10.1", "1.5.2.1", "linux", {"file1", "file2"}};
    PackageVersion v2 {"2.0.0", "10.1", "1.5.3.2", "linux", {"file1", "file2"}};
    PackageVersion v3 {"3.0.0", "10.1", "1.5.2.4", "linux", {"file1", "file2"}};

    // Mockup Package with versions
    Package package {"TestPackage", {"Author1", "Author2"}, "https://example.com", "https://github.com/example/test", "MIT", "Test package description", {"keyword1", "keyword2"}, {v1, v2, v3}};

    SECTION("Test with compatible versions") {
      ChuckVersion language_ver = ChuckVersion("1.5.2.4 (chai)"); // Mockup ChuckVersion
      ApiVersion api_ver = ApiVersion("10.1"); // Mockup ApiVersion
      string os = "linux"; // Mockup operating system

      // Get the latest compatible version
      auto latest = package.latest_version(os, language_ver, api_ver);

      // Assert that the latest version is v3
      REQUIRE(latest.has_value());
      REQUIRE(*latest == v3);
    }

    SECTION("Test with incompatible versions") {
      ChuckVersion language_ver = ChuckVersion("1.4.2.4 (nunchucks)"); // Mockup ChuckVersion
      ApiVersion api_ver = ApiVersion("10.1"); // Mockup ApiVersion
      string os = "linux"; // Mockup operating system

      // Get the latest compatible version
      auto latest = package.latest_version(os, language_ver, api_ver);

      // Assert that no version is returned
      REQUIRE_FALSE(latest.has_value());
    }
}
