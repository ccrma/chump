#include "manager.h"
#include "exec.h"

#include <filesystem>
#include <catch2/catch_test_macros.hpp>

std::string manifest_url = "https://ccrma.stanford.edu/~nshaheed/chump/manifest.json";

TEST_CASE("Integration Tests - install/update/uninstall") {
  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion,
                           langVersion, "linux", manifest_url, false);

  // install package

  m->install("TestPackage");

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  // Update Package
  ckVersion = ChuckVersion("1.5.2.6");
  langVersion = ApiVersion("10.1");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux", manifest_url, false);

  m->update("TestPackage");

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  CHECK(fs::exists(installPath / "TestPackage" / "version.json"));

  // Uninstall Package
  m->uninstall("TestPackage");

  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}

TEST_CASE("Integration Tests - install/update/uninstall with dirs") {
  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion,
                           langVersion, "linux", manifest_url, false);

  // install package

  m->install("TestPackageDir");

  REQUIRE(fs::exists(installPath / "TestPackageDir"));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "version.json"));

  // Update Package
  ckVersion = ChuckVersion("1.5.2.6");
  langVersion = ApiVersion("10.1");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux", manifest_url, false);

  m->update("TestPackageDir");

  REQUIRE(fs::exists(installPath / "TestPackageDir"));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "dir" ));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "dir" / "hello.ck"));
  CHECK(fs::exists(installPath / "TestPackageDir" / "version.json"));

  // Add a file not associated with the package

  // Create and open a text file
  std::ofstream extraFile(installPath / "TestPackageDir" / "trespassing.txt");
  // Write to the file
  extraFile << "I'm not supposed to be here";
  extraFile.close();

  // Uninstall Package
  m->uninstall("TestPackageDir");

  REQUIRE(fs::exists(installPath / "TestPackageDir"));
  REQUIRE_FALSE(fs::exists(installPath / "TestPackageDir" / "dir"));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "trespassing.txt"));
}

TEST_CASE("Integration Tests - install specific version") {

  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion,
                           langVersion, "linux", manifest_url, false);

  m->install("TestPackage=1.0.0");

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

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion,
                           langVersion, "linux", manifest_url, false);

  // install package
  m->install("DoesNotExist");

  REQUIRE_FALSE(fs::exists(installPath / "DoesNotExist"));
}


TEST_CASE("Integration Tests - file doesn't exist") {
  // If a url points to a file that doesn't exist, the package installation should fail

  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::cout << "install path: " << installPath << std::endl;

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion,
                           langVersion, "linux", manifest_url, false);

  // install specific version of package (with file that can't be found)
  m->install("TestPackage=0.9.0");

  // Because there is a missing file, the package wasn't installed
  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}

TEST_CASE("Integration Tests - bad version input") {

  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion,
                           langVersion, "linux", manifest_url, false);

  // This is incorrect
  m->install("TestPackage=vfjdklsaf1.0.0");

  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}

TEST_CASE("Integration Tests - install twice") {
  // If a url points to a file that doesn't exist, the package installation should fail

  // reference: https://stackoverflow.com/questions/52912981/create-a-unique-temporary-directory
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion,
                           langVersion, "linux", manifest_url, false);

  // install specific version of package (with file that can't be found)
  m->install("TestPackage");

  // Because there is a missing file, the package wasn't installed
  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  // installing again won't work
  bool result = m->install("TestPackage");
  REQUIRE_FALSE(result);
}

TEST_CASE("Integration Tests - update manifest") {
  fs::path installPath {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directories(installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager* m = new Manager(dataPath, installPath, ckVersion,
                           langVersion, "linux", manifest_url, false);

  REQUIRE(m->update_manifest());
}
