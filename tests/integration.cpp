#include "manager.h"
#include "exec.h"

#include <filesystem>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Integration Tests") {

  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/packages.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux");

  // install package
  std::cout << "the install dir: " << installPath << std::endl;

  std::vector<std::string> args = {"TestPackage"};
  execCommand("install", args, m);

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  // Update Package
  ckVersion = ChuckVersion("1.5.2.6");
  langVersion = ApiVersion("10.1");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux");

  execCommand("update", args, m);

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  CHECK(fs::exists(installPath / "TestPackage" / "version.json"));

  // Uninstall Package
  execCommand("uninstall", args, m);

  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}
