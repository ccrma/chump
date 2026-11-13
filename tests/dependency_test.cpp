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

// std::string manifest_url_deps =
//     "https://ccrma.stanford.edu/~nshaheed/chump/manifest.json";
std::string manifest_url_deps = "";
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
  REQUIRE_FALSE(m->uninstall_with_deps({"A"}, false));

  validateInstall(installPath, "A");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("Force uninstall package A");
  // This should not succeed because B and C depend on A
  REQUIRE_FALSE(m->uninstall_with_deps({"A"}, false));
  validateInstall(installPath, "A");

  // now force it to uninstall
  REQUIRE(m->uninstall_with_deps({"A"}, true));
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
  REQUIRE(m->uninstall_with_deps({"A"}, false));

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
  REQUIRE_FALSE(m->uninstall_with_deps({"A"}, false));

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
  m->uninstall_with_deps({"A"}, true);

  validateUninstall(installPath, "A");
  validateUninstall(installPath, "B");
  validateInstall(installPath, "C");

  INFO("Attempt to uninstall package C (without --force)");
  REQUIRE(m->uninstall_with_deps({"C"}, false));

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
  REQUIRE(m->uninstall_with_deps({"C"}, true));

  // A & B should remain installed because D depends on them
  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateUninstall(installPath, "C");
}

TEST_CASE("Dependency Tests - uninstall multiple pkgs") {
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

  INFO("Attempt to uninstall packages C & D");
  REQUIRE(m->uninstall_with_deps({"C", "D"}, true));

  // Everything should be uninstalled
  validateUninstall(installPath, "A");
  validateUninstall(installPath, "B");
  validateUninstall(installPath, "C");
  validateUninstall(installPath, "D");
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
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  std::cout << "local test\n";

  Manager *m = makeLocalTestManager(installPath);

  INFO("Installing");
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
  runInstaller(m->install_local_with_deps(
                   localPkgDefn, localDataPath / "version_missing_dep.json",
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

//-----------------------------------------------------------------------------
// Dependency resolution edge cases
//
// Packages being tested:
//   LangMax     - 1.0.0, and 2.0.0 which has language_version_max 1.5.2.4
//   DepRangeMin - depends on E >= 1.0.0
//   DepRangeMax - depends on E <= 1.0.0
//   NeedsA3     - depends on A >= 3.0.0 (A 3.0.0 requires api 10.1)
//   BrokenDep   - depends on A and NOEXIST (not in the manifest)
//   OSPick      - 1.0.0 is linux, 2.0.0 is mac-only
//-----------------------------------------------------------------------------
Manager *makeDepManager(fs::path installPath,
                        ChuckVersion ckVersion = ChuckVersion("1.5.2.1"),
                        ApiVersion apiVersion = ApiVersion("9.1")) {
  return new Manager(dataPath, installPath, ckVersion, apiVersion, "linux",
                     X86_64, manifest_url_deps, false);
}

bool isUserInstalled(fs::path installPath, string package) {
  return openVer(installPath / package / "version.json").user_installed;
}

// run the installer to completion (accepting every prompt) and return the
// final result without asserting on it
install_with_deps_result *
runInstallerResult(Generator<install_with_deps_result *> &installer) {
  auto res = installer();
  while (!res->finished) {
    res->cont = true;
    res = installer();
  }
  return res;
}

//-------------------------------- version selection --------------------------

TEST_CASE("Dependency Tests - picks newest version compatible with API") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  // A 3.0.0 is the newest, but requires api 10.1. With api 9.1 the newest
  // compatible version is 2.0.0.
  Manager *m = makeDepManager(installPath, ChuckVersion("1.5.2.6"));
  runInstaller(m->install_with_deps({"A"}));

  validateInstall(installPath, "A", "2.0.0");
}

TEST_CASE("Dependency Tests - respects language_version_max") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  SECTION("chuck version above max picks older version") {
    Manager *m = makeDepManager(installPath, ChuckVersion("1.5.2.5"));
    runInstaller(m->install_with_deps({"LangMax"}));

    validateInstall(installPath, "LangMax", "1.0.0");
  }

  SECTION("chuck version within max picks newest version") {
    Manager *m = makeDepManager(installPath, ChuckVersion("1.5.2.1"));
    runInstaller(m->install_with_deps({"LangMax"}));

    validateInstall(installPath, "LangMax", "2.0.0");
  }
}

TEST_CASE("Dependency Tests - respects package OS") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  // OSPick 2.0.0 only exists for mac, so on linux 1.0.0 should be chosen
  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"OSPick"}));

  validateInstall(installPath, "OSPick", "1.0.0");
}

TEST_CASE("Dependency Tests - dependency with only version_min") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  // E >= 1.0.0 should resolve to the newest E (2.0.0)
  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"DepRangeMin"}));

  validateInstall(installPath, "DepRangeMin");
  validateInstall(installPath, "E", "2.0.0");
}

TEST_CASE("Dependency Tests - dependency with only version_max") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  // E <= 1.0.0 should resolve to the newest E within range (1.0.0, not 0.1.0)
  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"DepRangeMax"}));

  validateInstall(installPath, "DepRangeMax");
  validateInstall(installPath, "E", "1.0.0");
}

//-------------------------------- unsatisfiable installs ---------------------

TEST_CASE("Dependency Tests - dependency has no compatible version") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  // NeedsA3 needs A >= 3.0.0, which needs api 10.1 (we have 9.1)
  Manager *m = makeDepManager(installPath, ChuckVersion("1.5.2.6"));
  runInstaller(m->install_with_deps({"NeedsA3"}), false);

  validateUninstall(installPath, "NeedsA3");
  validateUninstall(installPath, "A");
}

TEST_CASE("Dependency Tests - dependency missing from manifest") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  // BrokenDep depends on A (fine) and NOEXIST (not in manifest). Nothing
  // should be installed, including A.
  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"BrokenDep"}), false);

  validateUninstall(installPath, "BrokenDep");
  validateUninstall(installPath, "A");
}

TEST_CASE("Dependency Tests - conflicting requested packages") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  // G needs E == 1.0.0 and H needs E == 2.0.0, so they can't both be
  // installed
  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"G", "H"}), false);

  validateUninstall(installPath, "G");
  validateUninstall(installPath, "H");
  validateUninstall(installPath, "E");
}

TEST_CASE("Dependency Tests - one requested package does not exist") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"A", "NOEXIST"}), false);

  validateUninstall(installPath, "A");
}

//-------------------------------- already installed --------------------------

TEST_CASE("Dependency Tests - install already-installed package") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"E=1.0.0"}));
  validateInstall(installPath, "E", "1.0.0");

  // installing (not updating) an installed package should fail and leave
  // it alone
  m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"E"}), false);

  validateInstall(installPath, "E", "1.0.0");
}

TEST_CASE("Dependency Tests - update already up-to-date package") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"E"}));
  validateInstall(installPath, "E", "2.0.0");

  // there is nothing to update, so the solver produces no transaction steps
  m = makeDepManager(installPath);
  auto installer = m->install_with_deps({"E"}, true);
  auto res = runInstallerResult(installer);

  REQUIRE(res->finished);
  validateInstall(installPath, "E", "2.0.0");
}

//-------------------------------- user-installed tracking --------------------

TEST_CASE("Dependency Tests - requested packages are user-installed and "
          "dependencies are not") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"C"}));

  REQUIRE(isUserInstalled(installPath, "C"));
  REQUIRE_FALSE(isUserInstalled(installPath, "B"));
  REQUIRE_FALSE(isUserInstalled(installPath, "A"));
}

TEST_CASE("Dependency Tests - dependency is changed without prompting") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"F=1.0.0"}));
  validateInstall(installPath, "E", "2.0.0");
  REQUIRE_FALSE(isUserInstalled(installPath, "E"));

  // G needs E == 1.0.0. E is only a dependency, so it should be downgraded
  // without asking the user
  m = makeDepManager(installPath);
  auto installer = m->install_with_deps({"G"});
  auto res = installer();

  REQUIRE(res->finished);
  REQUIRE(res->success);

  validateInstall(installPath, "G");
  validateInstall(installPath, "F", "1.0.0");
  validateInstall(installPath, "E", "1.0.0");

  // still a dependency after the downgrade
  REQUIRE_FALSE(isUserInstalled(installPath, "E"));
}

//-------------------------------- user prompts -------------------------------

TEST_CASE("Dependency Tests - prompts before upgrading user-installed "
          "package") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"E=1.0.0"}));
  REQUIRE(isUserInstalled(installPath, "E"));

  // H needs E == 2.0.0, and E was installed by the user
  m = makeDepManager(installPath);
  auto installer = m->install_with_deps({"H"});
  auto res = installer();

  REQUIRE_FALSE(res->finished);
  REQUIRE(res->action_type == UPGRADE);
  REQUIRE(res->name == "E");
  REQUIRE(res->old_version == "1.0.0");
  REQUIRE(res->new_version == "2.0.0");

  SECTION("accept upgrade") {
    res->cont = true;
    res = installer();

    REQUIRE(res->finished);
    REQUIRE(res->success);

    validateInstall(installPath, "H");
    validateInstall(installPath, "E", "2.0.0");

    // upgrading doesn't change who installed it
    REQUIRE(isUserInstalled(installPath, "E"));
  }

  SECTION("decline upgrade") {
    res->cont = false;
    res = installer();

    REQUIRE(res->finished);
    REQUIRE_FALSE(res->success);

    // nothing should have changed
    validateUninstall(installPath, "H");
    validateInstall(installPath, "E", "1.0.0");
  }
}

TEST_CASE("Dependency Tests - decline uninstalling user-installed package") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"G"}));

  // installing H requires removing G (see "conflicting dependencies")
  m = makeDepManager(installPath);
  auto installer = m->install_with_deps({"H"});
  auto res = installer();

  REQUIRE_FALSE(res->finished);
  REQUIRE(res->action_type == UNINSTALL);
  REQUIRE(res->name == "G");

  res->cont = false;
  res = installer();

  REQUIRE(res->finished);
  REQUIRE_FALSE(res->success);

  // nothing should have changed
  validateInstall(installPath, "G");
  validateInstall(installPath, "E", "1.0.0");
  validateUninstall(installPath, "H");
}

//-------------------------------- uninstall ----------------------------------

TEST_CASE("Dependency Tests - uninstall removes orphaned dependencies") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"C"}));

  m = makeDepManager(installPath);
  REQUIRE(m->uninstall_with_deps({"C"}, false));

  // A and B were only installed for C, so they should be removed too
  validateUninstall(installPath, "C");
  validateUninstall(installPath, "B");
  validateUninstall(installPath, "A");
}

TEST_CASE("Dependency Tests - uninstall keeps user-installed dependencies") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"A"}));

  // C pulls in B; A is already installed (by the user)
  m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"C"}));

  REQUIRE(isUserInstalled(installPath, "A"));
  REQUIRE_FALSE(isUserInstalled(installPath, "B"));

  m = makeDepManager(installPath);
  REQUIRE(m->uninstall_with_deps({"C"}, false));

  validateUninstall(installPath, "C");
  validateUninstall(installPath, "B");
  // A was explicitly installed by the user, so it stays
  validateInstall(installPath, "A");
}

TEST_CASE("Dependency Tests - uninstall package that is not installed") {
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  Manager *m = makeDepManager(installPath);
  runInstaller(m->install_with_deps({"A"}));

  m = makeDepManager(installPath);
  REQUIRE_FALSE(m->uninstall_with_deps({"C"}, false));

  // unrelated packages are left alone
  validateInstall(installPath, "A");
}
