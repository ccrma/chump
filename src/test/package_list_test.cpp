#include "package_list.h"
#include <catch2/catch_test_macros.hpp>

using std::optional;

TEST_CASE("Load db file", "[PackageList]") {
  std::string path = "./data/test-package-list.json";
  PackageList pkglist = PackageList(path);

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

  optional<Package> got = pkglist.lookup("Butt");


  REQUIRE(got == want);

  got = pkglist.lookup("Butt", "1.0");
  REQUIRE(got == want);


  got = pkglist.lookup("NotReal");
  REQUIRE_FALSE(got);
}
