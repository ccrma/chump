#include "exec.h"
#include "fetch.h"
#include "manager.h"
#include "package.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>

// std::string manifest_url =
//     "https://ccrma.stanford.edu/~nshaheed/chump/manifest.json";

// Don't update manifest. This is maybe bad since it's an integration test, but this takes
// so long
std::string manifest_url = "";

bool install(Manager *m, string pkg) {
  INFO("install, fetch to tmp dir");
  auto tmp = m->fetch_to_tmp(pkg, false);

  if (tmp) {
    INFO("install from tmp dir: " << tmp.value());
    return m->install(pkg, tmp.value());
  } else {
    INFO("install no tmp dir returned");
    return false;
  }
}

void runInstallerIntegration(Generator<install_with_deps_result *> installer,
                             bool succeed = true) {
  auto res = installer();
  while (!res->finished) {
    res->cont = true;
    res = installer();
  }
  REQUIRE(succeed == res->success);
}

TEST_CASE("Integration Tests - install/update/uninstall") {
  // setDebugEnabled(true);
  fs::path installPath = makeTempDir();
  INFO("install path: " << installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // install package
  INFO("installing TestPackage");
  REQUIRE(install(m, "TestPackage"));

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  // Update Package
  ckVersion = ChuckVersion("1.5.2.6");
  langVersion = ApiVersion("10.1");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url, false);

  INFO("updating TestPackage");
  runInstallerIntegration(
      m->install_with_deps({"TestPackage"}, true)); // set update to true

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "_data" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  // Uninstall Package
  m->uninstall("TestPackage", false);

  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}

TEST_CASE("Integration Tests - install/update/uninstall with dirs") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // install package

  install(m, "TestPackageDir");

  REQUIRE(fs::exists(installPath / "TestPackageDir"));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "version.json"));

  // Update Package
  ckVersion = ChuckVersion("1.5.2.6");
  langVersion = ApiVersion("10.1");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url, false);

  runInstallerIntegration(
      m->install_with_deps({"TestPackageDir"}, true)); // set update to true

  REQUIRE(fs::exists(installPath / "TestPackageDir"));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "dir"));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "dir" / "hello.ck"));
  CHECK(fs::exists(installPath / "TestPackageDir" / "version.json"));

  // Add a file not associated with the package

  // Create and open a text file
  std::ofstream extraFile(installPath / "TestPackageDir" / "trespassing.txt");
  // Write to the file
  extraFile << "I'm not supposed to be here";
  extraFile.close();

  // Uninstall Package
  m->uninstall("TestPackageDir", false);

  REQUIRE(fs::exists(installPath / "TestPackageDir"));
  REQUIRE_FALSE(fs::exists(installPath / "TestPackageDir" / "dir"));
  REQUIRE(fs::exists(installPath / "TestPackageDir" / "trespassing.txt"));
}

TEST_CASE("Integration Tests - install specific version") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  install(m, "TestPackage=1.0.0");

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));
}

TEST_CASE("Integration Tests - package doesn't exist") {
  // TODO: this is a bit of a jank way to test this - is there a way to capture
  // the failure state at the end of this?
  // Follow up: should I throw an exception for a package not existing?
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // install package
  install(m, "DoesNotExist");

  REQUIRE_FALSE(fs::exists(installPath / "DoesNotExist"));
}

TEST_CASE("Integration Tests - file doesn't exist") {
  // If a url points to a file that doesn't exist, the package installation
  // should fail
  fs::path installPath = makeTempDir();

  std::cout << "install path: " << installPath << std::endl;

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // install specific version of package (with file that can't be found)
  install(m, "TestPackage=0.9.0");

  // Because there is a missing file, the package wasn't installed
  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}

TEST_CASE("Integration Tests - bad version input") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // This is incorrect
  install(m, "TestPackage=vfjdklsaf1.0.0");

  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}

TEST_CASE("Integration Tests - install twice") {
  // If a url points to a file that doesn't exist, the package installation
  // should fail
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // install specific version of package (with file that can't be found)
  install(m, "TestPackage");

  // Because there is a missing file, the package wasn't installed
  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  // installing again won't work
  auto tmp = m->fetch_to_tmp("TestPackage", false);
  bool result = m->install("TestPackage", tmp.value());
  REQUIRE_FALSE(result);
}

TEST_CASE("Integration Tests - update manifest") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // should update in constructor so won't change anything
  REQUIRE_FALSE(m->update_manifest());
}

TEST_CASE("Integration Test - wrong checksum") {
  Fetch f;
  Package p;
  PackageVersion v;

  fs::path tmpPath = makeTempDir();

  // This will fail due to an incorrect checksum being provided
  bool result = f.fetch("https://ccrma.stanford.edu/~nshaheed/chugins/"
                        "TestPackage/1.0.0/hello.ck",
                        "./", p, v, tmpPath, PACKAGE_FILE, "1234");
  REQUIRE_FALSE(result);
}

TEST_CASE("Integration Test - zip files") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // install package

  install(m, "TestPackageZip");

  REQUIRE(fs::exists(installPath / "TestPackageZip"));
  REQUIRE(fs::exists(installPath / "TestPackageZip" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackageZip" / "version.json"));
  REQUIRE_FALSE(
      fs::exists(installPath / "TestPackageZip" / "test_package_zip.zip"));

  // Update Package
  ckVersion = ChuckVersion("1.5.2.6");
  langVersion = ApiVersion("10.1");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url, false);

  runInstallerIntegration(
      m->install_with_deps({"TestPackageZip"}, true)); // set update to true

  REQUIRE(fs::exists(installPath / "TestPackageZip"));
  REQUIRE(fs::exists(installPath / "TestPackageZip" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackageZip" / "_data" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackageZip" / "version.json"));
  REQUIRE_FALSE(
      fs::exists(installPath / "TestPackageZip" / "test_package_zip.zip"));
}

TEST_CASE("Integration Test - uninstall with missing file") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // install package

  install(m, "TestPackage");

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  // remove file, which will hopefully not screw things up
  fs::remove(installPath / "TestPackage" / "hello.ck");

  // Uninstall Package
  m->uninstall("TestPackage", false);

  // Validate that everything got deleted properly
  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}

TEST_CASE("Integration Test - update with missing file") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // install package

  install(m, "TestPackage");

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  // remove file, which will hopefully not screw things up
  fs::remove(installPath / "TestPackage" / "hello.ck");

  // Update Package
  ckVersion = ChuckVersion("1.5.2.6");
  langVersion = ApiVersion("10.1");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url, false);

  runInstallerIntegration(
      m->install_with_deps({"TestPackage"}, true)); // set update to true

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "_data" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));
}

TEST_CASE("Integration Test - force uninstall") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.0");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url, false);

  // install package
  INFO("Installating package");
  install(m, "TestPackage");

  REQUIRE(fs::exists(installPath / "TestPackage"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "hello.ck"));
  REQUIRE(fs::exists(installPath / "TestPackage" / "version.json"));

  INFO("Force-uninstalling package");
  // Uninstall Package
  m->uninstall("TestPackage", true);

  REQUIRE_FALSE(fs::exists(installPath / "TestPackage"));
}
