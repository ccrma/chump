#include "manager.h"
#include "exec.h"

#include <filesystem>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Integration Tests - install/update/uninstall") {
  std::cerr << "beginning\n";
  
  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::cerr << "tmp dir mad: " << installPath << std::endl;

  std::string dataPath = "../data/packages.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux");

  std::cerr <<"Manager made\n";

  // install package
  std::cout << "the install dir: " << installPath << std::endl;

  std::vector<std::string> args = {"TestPackage"};
  std::cerr << "executing install\n";
  execCommand("install", args, m);
  std::cerr << "finish install\n";

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  // Update Package
  ckVersion = ChuckVersion("1.5.2.6");
  langVersion = ApiVersion("10.1");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux");

  std::cerr << "executing update\n";
  execCommand("update", args, m);
  std::cerr << "finishign install\n";

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  CHECK(fs::exists(installPath / "TestPackage" / "version.json"));

  // Uninstall Package

  std::cerr << "executing uninstall\n";
  execCommand("uninstall", args, m);
  std::cerr << "finishing uninstall\n";

  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}

TEST_CASE("Integration Tests - install specific version") {

  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/packages.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux");

  std::vector<std::string> args = {"TestPackage=1.0.0"};
  execCommand("install", args, m);

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));
}

TEST_CASE("Integration Tests - package doesn't exist") {
  // TODO: this is a bit of a jank way to test this - is there a way to capture
  // the failure state at the end of this?
  // Follow up: should I throw an exception for a package not existing?

  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/packages.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux");

  // install package
  std::vector<std::string> args = {"DoesNotExist"};
  execCommand("install", args, m);

  REQUIRE_FALSE(fs::exists(installPath / "DoesNotExist"));
}


TEST_CASE("Integration Tests - file doesn't exist") {
  // If a url points to a file that doesn't exist, the package installation should fail

  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::cout << "install path: " << installPath << std::endl;

  std::string dataPath = "../data/packages.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux");

  // install specific version of package (with file that can't be found)
  std::vector<std::string> args = {"TestPackage=0.9.0"}; // do this once I get the file thing working

  // install
  execCommand("install", args, m);

  // Because there is a missing file, the package wasn't installed
  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}

TEST_CASE("Integration Tests - bad version input") {

  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/packages.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux");

  // This is incorrect
  std::vector<std::string> args = {"TestPackage=vfjdklsaf1.0.0"};
  execCommand("install", args, m);

  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}
