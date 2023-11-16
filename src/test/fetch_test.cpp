#include "fetch.h"
#include "package.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("fetch url", "[fetchurl]") {
  Fetch f;

  std::string path = "./testdata/test.pkg";

  Package want = {"John", "PackageA", "1.0", "9.0", "http://example.com", "http://repo.com", "specA", {"AuthorA", "AuthorB"}, "MIT", "DescriptionA", {"KeywordA", "KeywordB"}};

  
  Package got = f.fetch(path);

  std::cout << want << std::endl << std::endl << std::endl << std::endl;

  REQUIRE(got == want);
}
