#include "exec.h"
#include "fetch.h"
#include "manager.h"
#include "package.h"
#include "util.h"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

std::string manifest_url_deps =
    "https://ccrma.stanford.edu/~nshaheed/chump/manifest.json";
std::string dataPath = "../data/manifest.json";

InstalledVersion openVer(fs::path path) {
  std::ifstream f(path);

  // TODO better error checks
  if (!f.good()) {
    throw std::invalid_argument("Unable to open Package List \"" +
                                path.string() + "\"");
  }

  json data = json::parse(f);
  f.close();

  InstalledVersion ver = data.get<InstalledVersion>();

  return ver;
}

void validateInstall(fs::path installPath, string package,
                     string version = "") {
  INFO("Validating installation of " << package);
  REQUIRE(fs::exists(installPath / package));
  REQUIRE(fs::exists(installPath / package / "version.json"));

  if (version != "") {
    auto ver = openVer(installPath / package / "version.json");
    REQUIRE(ver.getVersionString() == version);
  }
}

void validateUninstall(fs::path installPath, string package) {
  REQUIRE_FALSE(fs::exists(installPath / package));
}

void runInstaller(Generator<install_with_deps_result *> installer,
                  bool succeed = true) {
  auto res = installer();

  while (!res->finished) {
    res->cont = true;
    res = installer();
  }

  REQUIRE(succeed == res->success);
}

void printRes(install_with_deps_result *res) {
  std::cout << "~~~~~~~~ Res Debug Print ~~~~~~~~\n";
  std::cout << "Res Name: " << res->name << std::endl;

  string action;

  switch (res->action_type) {
  case (NONE):
    action = "NONE";
    break;
  case (INSTALL):
    action = "INSTALL";
    break;
  case (UNINSTALL):
    action = "UNINSTALL";
    break;
  case (UPGRADE):
    action = "UPGRADE";
    break;
  }

  std::cout << "Action Type: " << action << std::endl;

  std::cout << "Finished: " << res->finished << std::endl;
  std::cout << "Success: " << res->success << std::endl;
  std::cout << "Continue: " << res->cont << std::endl;
  std::cout << "old_version: \"" << res->old_version << "\"\n";
  std::cout << "new_version: \"" << res->new_version << "\"\n";
  std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
}

TEST_CASE("Dependency Tests - install with dependencies") {
  fs::path installPath = makeTempDir();

  ChuckVersion ckVersion = ChuckVersion("1.5.5.9");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package

  // REQUIRE(m->install_with_deps({"C"}));
  runInstaller(m->install_with_deps({"C"}));

  // B depends on A so A should have also been installed
  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateInstall(installPath, "C");
}

TEST_CASE("Dependency Tests - tries to uninstall not-installed package") {
  fs::path installPath = makeTempDir();

  ChuckVersion ckVersion = ChuckVersion("1.5.5.9");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package

  runInstaller(m->install_with_deps({"NOEXIST"}), false);
}

TEST_CASE("Dependency Tests - redundant deps") {
  fs::path installPath = makeTempDir();

  ChuckVersion ckVersion = ChuckVersion("1.5.5.9");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package

  runInstaller(m->install_with_deps({"D"}));

  // B depends on A so A should have also been installed
  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateInstall(installPath, "D");

  // should not be installed
  validateUninstall(installPath, "C");
}

// TODO test to validate it gets the proper package version for the proper
// language version
TEST_CASE("Dependency Tests - install with dependencies (old chuck version)") {
  fs::path installPath = makeTempDir();

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package

  // m->install_with_deps("B");
  runInstaller(m->install_with_deps({"C"}));

  // C depends on A & B should they should have also been installed
  validateInstall(installPath, "A", "1.0.0");
  validateInstall(installPath, "B", "1.0.0");
  validateInstall(installPath, "C", "1.0.0");
}

TEST_CASE("Dependency Tests - incompatible API version") {
  fs::path installPath = makeTempDir();

  ChuckVersion ckVersion = ChuckVersion("1.5.2.6");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // attempt install package, but it has incompatible API version
  runInstaller(m->install_with_deps({"A=3.0.0"}), false);
}

// TEST need to downgrade a package
TEST_CASE("Dependency Tests - downgrade package") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  std::cout << "IN teh test\n";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  INFO("Set up Manager");

  // install package

  runInstaller(m->install_with_deps({"E=2.0.0"}));
  validateInstall(installPath, "E", "2.0.0");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  runInstaller(m->install_with_deps({"F=2.0.0"}));

  validateInstall(installPath, "F", "2.0.0");
  validateInstall(installPath, "E", "1.0.0");
}

// TEST need to upgrade a package
TEST_CASE("Dependency Tests - upgrade package") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  INFO("Set up Manager");

  // install package

  runInstaller(m->install_with_deps({"E=0.1.0"}));

  validateInstall(installPath, "E", "0.1.0");

  INFO(installPath / "E" / "version.json" << "\n");

  // TODO how to do this without making new manager?
  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  runInstaller(m->install_with_deps({"F=2.0.0"}));

  validateInstall(installPath, "F", "2.0.0");
  validateInstall(installPath, "E", "1.0.0");
}

// TEST need to uninstall a package
TEST_CASE("Dependency Tests - conflicting dependencies") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);
  std::cout << "IN teh test\n";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  INFO("Set up Manager");

  // install package
  runInstaller(m->install_with_deps({"G"}));

  validateInstall(installPath, "G");
  validateInstall(installPath, "E", "1.0.0");

  INFO(installPath / "E" / "version.json" << "\n");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("New Manager Created");

  auto installer = m->install_with_deps({"H"});
  auto res = installer();

  // printRes(res);
  REQUIRE_FALSE(res->finished);           // reached conflict with install
  REQUIRE(res->action_type == UNINSTALL); // will uninstall G
  REQUIRE(res->name == "G");
  res->cont = true;

  // because E is a dependency, and no user-installed, it will get removed
  // automatically
  res = installer();

  REQUIRE(res->finished);
  REQUIRE(res->success);

  validateInstall(installPath, "H");
  validateInstall(installPath, "E", "2.0.0");
}

TEST_CASE("Dependency Tests - uninstall dependant pacakge") {
  TC::off();
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  INFO("Set up Manager");
  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package
  INFO("Installing package C");
  runInstaller(m->install_with_deps({"C"}));

  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateInstall(installPath, "C");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("Attempt to uninstall package A");
  // This should not succeed because B and C depend on A
  REQUIRE_FALSE(m->uninstall_with_deps("A", false));

  validateInstall(installPath, "A");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("Force uninstall package A");
  // This should not succeed because B and C depend on A
  REQUIRE_FALSE(m->uninstall_with_deps("A", false));
  validateInstall(installPath, "A");

  // now force it to uninstall
  REQUIRE(m->uninstall_with_deps("A", true));
  validateUninstall(installPath, "A");
}

// TEST need to upgrade a package
TEST_CASE("Dependency Tests - uninstall a package") {
  TC::off();
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  INFO("Set up Manager");
  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package
  INFO("Installing package A");
  runInstaller(m->install_with_deps({"A"}));

  validateInstall(installPath, "A");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("Attempt to uninstall package A");
  // This should succeed because there are no dependencies for it
  REQUIRE(m->uninstall_with_deps("A", false));

  validateUninstall(installPath, "A");
}

TEST_CASE(
    "Dependency Tests - uninstall package with user-installed dependencies") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  INFO("Set up Manager");
  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package
  INFO("Installing packages A & C");
  runInstaller(m->install_with_deps({"A", "C"}));

  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateInstall(installPath, "C");

  // need new manager to go updated repos
  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("Attempt to uninstall package A");
  // This should not succeed because B and C depend on A
  REQUIRE_FALSE(m->uninstall_with_deps("A", false));

  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateInstall(installPath, "C");
}

TEST_CASE("Dependency Tests - uninstall package with user-installed "
          "dependencies (with --force)") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  INFO("Set up Manager");
  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package
  INFO("Installing packages A & C");
  runInstaller(m->install_with_deps({"A", "C"}));

  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateInstall(installPath, "C");

  // need new manager to go updated repos
  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("Attempt to uninstall package A");
  // This should not succeed because B and C depend on A
  // REQUIRE_FALSE(m->uninstall_with_deps("A", true));
  m->uninstall_with_deps("A", true);

  validateUninstall(installPath, "A");
  validateUninstall(installPath, "B");
  validateInstall(installPath, "C");

  INFO("Attempt to uninstall package C (without --force)");
  REQUIRE(m->uninstall_with_deps("C", false));

  validateUninstall(installPath, "C");
}

TEST_CASE(
    "Dependency Tests - uninstall package where dep is dep for another pkg") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  INFO("Set up Manager");
  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package
  INFO("Installing packages C & D");
  runInstaller(m->install_with_deps({"C", "D"}));

  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateInstall(installPath, "C");
  validateInstall(installPath, "D");

  // need new manager to go updated repos
  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("Attempt to uninstall package C");
  REQUIRE(m->uninstall_with_deps("C", true));

  // A & B should remain installed because D depends on them
  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateUninstall(installPath, "C");
}

TEST_CASE("Dependency Tests - will install auto update?") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  SECTION("A") {
    INFO("Set up Manager");
    Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                             "linux", X86_64, manifest_url_deps, false);

    // install package
    INFO("Installing package E (1.0.0)");
    runInstaller(m->install_with_deps({"E=1.0.0"}));

    validateInstall(installPath, "E", "1.0.0");

    m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                    X86_64, manifest_url_deps, false);

    INFO("Should update E to 2.0.0");
    runInstaller(m->install_with_deps({"E"}, true));

    validateInstall(installPath, "E", "2.0.0");
  }

  SECTION("B") {
    INFO("Set up Manager");
    Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                             "linux", X86_64, manifest_url_deps, false);

    // install package
    INFO("Installing package F (1.0.0)");
    runInstaller(m->install_with_deps({"F=1.0.0"}));

    validateInstall(installPath, "F", "1.0.0");
    validateInstall(installPath, "E", "2.0.0");

    m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                    X86_64, manifest_url_deps, false);

    INFO("Upgrade F to 2.0.0");
    // This should not succeed because B and C depend on A
    runInstaller(m->install_with_deps({"F"}, true));

    validateInstall(installPath, "F", "2.0.0");
    validateInstall(installPath, "E", "1.0.0");
  }
}

//-----------------------------------------------------------------------------
// install_local_with_deps tests
// files are in data/local/:
//   package.json              - package definition for "LocalPkg"
//   version_no_deps.json      - version file with no dependencies
//   version_deps.json         - depends on B (which depends on A)
//   version_pinned_dep.json   - depends on E, pinned to exactly 1.0.0
//   version_missing_dep.json  - depends on NOEXIST (not in the manifest)
//   LocalPkg.zip              - package contents (hello.ck)
//-----------------------------------------------------------------------------
const fs::path localDataPath = "../data/local";
const fs::path localPkgDefn = localDataPath / "package.json";
const fs::path localPkgZip = localDataPath / "LocalPkg.zip";

Manager *makeLocalTestManager(fs::path installPath) {
  return new Manager("../data/manifest.json", installPath,
                     ChuckVersion("1.5.2.1"), ApiVersion("9.1"), "linux",
                     X86_64, manifest_url_deps, false);
}

void validateLocalInstall(fs::path installPath) {
  validateInstall(installPath, "LocalPkg", "1.0.0");
  REQUIRE(fs::exists(installPath / "LocalPkg" / "hello.ck"));

  // install-local packages are always user-installed
  auto ver = openVer(installPath / "LocalPkg" / "version.json");
  REQUIRE(ver.user_installed);
}

TEST_CASE("Dependency Tests - install local package (no dependencies)") {
  // setDebugEnabled(true);
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  std::cout << "locla test\n";

  Manager *m = makeLocalTestManager(installPath);

  runInstaller(m->install_local_with_deps(
      localPkgDefn, localDataPath / "version_no_deps.json", localPkgZip));

  validateLocalInstall(installPath);
}

TEST_CASE("Dependency Tests - install local package with dependencies") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeLocalTestManager(installPath);

  // LocalPkg depends on B, which depends on A
  runInstaller(m->install_local_with_deps(
      localPkgDefn, localDataPath / "version_deps.json", localPkgZip));

  validateLocalInstall(installPath);
  validateInstall(installPath, "A");
  validateInstall(installPath, "B");

  // dependencies were pulled in automatically, not requested by the user
  REQUIRE_FALSE(openVer(installPath / "A" / "version.json").user_installed);
  REQUIRE_FALSE(openVer(installPath / "B" / "version.json").user_installed);

  // unrelated packages should not be installed
  validateUninstall(installPath, "C");
}

TEST_CASE("Dependency Tests - install local package with pinned dependency") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeLocalTestManager(installPath);

  // E has 0.1.0, 1.0.0, and 2.0.0 available; LocalPkg pins it to 1.0.0
  runInstaller(m->install_local_with_deps(
      localPkgDefn, localDataPath / "version_pinned_dep.json", localPkgZip));

  validateLocalInstall(installPath);
  validateInstall(installPath, "E", "1.0.0");
}

TEST_CASE("Dependency Tests - install local package with dependencies "
          "already installed") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeLocalTestManager(installPath);

  INFO("Installing B (and A) from the manifest");
  runInstaller(m->install_with_deps({"B"}));
  validateInstall(installPath, "A");
  validateInstall(installPath, "B");

  // need new manager to get updated repos
  m = makeLocalTestManager(installPath);

  INFO("Installing LocalPkg, whose dependencies are already satisfied");
  runInstaller(m->install_local_with_deps(
      localPkgDefn, localDataPath / "version_deps.json", localPkgZip));

  validateLocalInstall(installPath);
  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
}

TEST_CASE("Dependency Tests - install local package with missing dependency") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeLocalTestManager(installPath);

  // NOEXIST is not in the manifest, so dependency resolution fails and
  // the local package should not be installed
  runInstaller(m->install_local_with_deps(localPkgDefn,
                                          localDataPath /
                                              "version_missing_dep.json",
                                          localPkgZip),
               false);

  validateUninstall(installPath, "LocalPkg");
}

TEST_CASE("Dependency Tests - install local package with invalid arguments") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeLocalTestManager(installPath);
  fs::path pkgVer = localDataPath / "version_no_deps.json";

  SECTION("package definition does not exist") {
    runInstaller(m->install_local_with_deps(localDataPath / "noexist.json",
                                            pkgVer, localPkgZip),
                 false);
  }

  SECTION("package version does not exist") {
    runInstaller(m->install_local_with_deps(
                     localPkgDefn, localDataPath / "noexist.json", localPkgZip),
                 false);
  }

  SECTION("package zip does not exist") {
    runInstaller(m->install_local_with_deps(localPkgDefn, pkgVer,
                                            localDataPath / "noexist.zip"),
                 false);
  }

  SECTION("package zip is not a zip file") {
    runInstaller(m->install_local_with_deps(localPkgDefn, pkgVer, pkgVer),
                 false);
  }

  SECTION("package definition is malformed") {
    // a version file is valid json, but not a valid package definition
    runInstaller(m->install_local_with_deps(pkgVer, pkgVer, localPkgZip),
                 false);
  }

  validateUninstall(installPath, "LocalPkg");
}
