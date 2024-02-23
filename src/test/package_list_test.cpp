#include "package_list.h"
#include <catch2/catch_test_macros.hpp>

using std::optional;

TEST_CASE("Load db file", "[PackageList]") {
  std::string path = "./data/test-package-list.json";
  PackageList pkglist = PackageList(path);

  Package want = {"John", "PackageA", "1.0", "9.0", "http://example.com/butt.chug", "http://example.com", "http://repo.com", "specA", {"AuthorA", "AuthorB"}, "MIT", "DescriptionA", {"KeywordA", "KeywordB"}};

  optional<Package> got = pkglist.lookup("PackageA");

  REQUIRE(got == want);

  got = pkglist.lookup("PackageA", "1.0");
  REQUIRE(got == want);
}
