#include "package_list.h"
#include <catch2/catch_test_macros.hpp>

using std::optional;

TEST_CASE("Load db file", "[PackageList]") {
  std::string path = "./data/test-package-list.json";
  PackageList pkglist = PackageList(path, "linux");

  Package want;
  PackageVersion version;

  // Version initialization
  version.version = "1.0";
  version.api_version = "9.0";
  version.language_version = "1.5.2.1";
  version.os = "linux";
  version.files.push_back("https://ccrma.stanford.edu/~nshaheed/chugins/Hydra/linux/butt.chug");

  // Package initialization
  want.name = "Butt";
  want.authors = {"AuthorA", "AuthorB"};
  want.homepage = "http://example.com";
  want.repository = "http://repo.com";
  want.license = "MIT";
  want.description = "DescriptionA";
  want.keywords = {"KeywordA", "KeywordB"};
  want.versions.push_back(version);

  // Package want = {"John", "Butt", "1.0", "9.0", {{"linux", {"https://ccrma.stanford.edu/~nshaheed/chugins/Hydra/linux/butt.chug"}}}, "http://example.com", "http://repo.com", "specA", {"AuthorA", "AuthorB"}, "MIT", "DescriptionA", {"KeywordA", "KeywordB"}};

  optional<Package> got = pkglist.find_package("Butt");

  REQUIRE(got.has_value());

  REQUIRE(got.value().name == want.name);
  REQUIRE(got.value().versions.size() > 0);
}

TEST_CASE("Find Package") {
  std::string path = "./data/test-package-list.json";
  PackageList pkglist = PackageList(path, "linux");

  SECTION("Successfully find package") {
    optional<Package> pkg = pkglist.find_package("Butt");
    REQUIRE(pkg.value().name == "Butt");
  }

  SECTION("Can't find package") {
    optional<Package> pkg = pkglist.find_package("Fork");
    REQUIRE_FALSE(pkg);
  }
}

TEST_CASE("Find Package Version") {
  std::string path = "./data/test-package-list.json";
  PackageList pkglist = PackageList(path, "linux");

  SECTION("Successfully find package") {
    optional<PackageVersion> version = pkglist.find_latest_package_version("Butt");
    REQUIRE(version.value().version == "1.0.0");
  }

  SECTION("Can't find package") {
    optional<PackageVersion> version = pkglist.find_latest_package_version("Fork");
    REQUIRE_FALSE(version);
  }

  SECTION("Successfully find version") {
    optional<PackageVersion> version = pkglist.find_package_version("Butt", "1.0.0");
    REQUIRE(version.value().version == "1.0.0");
  }

  SECTION("Unsuccessfully find version") {
    optional<PackageVersion> version = pkglist.find_package_version("Butt", "1.2.0");
    REQUIRE_FALSE(version.has_value());
  }
}
