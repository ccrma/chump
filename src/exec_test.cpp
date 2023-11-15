#include "exec.h"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("parseArgs", "[parseargs]") {
  int argc = 4;
  const char * argv[] = {"programname", "a", "b", "c"};

  auto [cmd, args] = parseArgs(argc, argv);

  std::vector<std::string> want({"b", "c"});

  REQUIRE(cmd == "a");
  REQUIRE(args == want);
}
