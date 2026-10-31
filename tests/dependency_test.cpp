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

void validateInstall(fs::path installPath, string package, string version = "") {
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

TEST_CASE("Dependency Tests - install with dependencies") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.5.9");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package

  // m->installWithDeps("B");
  REQUIRE(m->installWithDeps({"C"}));

  // B depends on A so A should have also been installed
  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateInstall(installPath, "C");
}

TEST_CASE("Dependency Tests - redundant deps") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.5.9");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package

  REQUIRE(m->installWithDeps({"D"}));

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

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // install package

  // m->installWithDeps("B");
  REQUIRE(m->installWithDeps({"C"}));

  // C depends on A & B should they should have also been installed
  validateInstall(installPath, "A", "1.0.0");
  validateInstall(installPath, "B", "1.0.0");
  validateInstall(installPath, "C", "1.0.0");
}

TEST_CASE("Dependency Tests - incompatible API version") {
  fs::path installPath = makeTempDir();

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.6");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  // attempt install package, but it has incompatible API version
  REQUIRE_FALSE(m->installWithDeps({"A=3.0.0"}));
}

// TEST need to downgrade a package
TEST_CASE("Dependency Tests - downgrade package") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  std::cout << "IN teh test\n";

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  INFO("Set up Manager");

  // install package

  REQUIRE(m->installWithDeps({"E=2.0.0"}));
  validateInstall(installPath, "E", "2.0.0");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  REQUIRE(m->installWithDeps({"F=2.0.0"}));

  validateInstall(installPath, "F", "2.0.0");
  validateInstall(installPath, "E", "1.0.0");
}

// TEST need to upgrade a package
TEST_CASE("Dependency Tests - upgrade package") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  std::cout << "IN teh test\n";

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  INFO("Set up Manager");

  // install package

  REQUIRE(m->installWithDeps({"E=0.1.0"}));

  validateInstall(installPath, "E", "0.1.0");

  INFO(installPath / "E" / "version.json" << "\n");

  // TODO how to do this without making new manager?
  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  REQUIRE(m->installWithDeps({"F=2.0.0"}));

  validateInstall(installPath, "F", "2.0.0");
  validateInstall(installPath, "E", "1.0.0");
}

// TEST need to uninstall a package
TEST_CASE("Dependency Tests - conflicting dependencies") {
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);
  std::cout << "IN teh test\n";

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);

  INFO("Set up Manager");

  // install package
  REQUIRE(m->installWithDeps({"G"}));

  validateInstall(installPath, "G");
  validateInstall(installPath, "E", "1.0.0");

  INFO(installPath / "E" / "version.json" << "\n");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("New Manager Created");

  REQUIRE_FALSE(m->installWithDeps({"H"}));
}


TEST_CASE("Dependency Tests - uninstall dependant pacakge") {
  TC::off();
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  INFO("Set up Manager");
  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);


  // install package
  INFO("Installing package C");
  REQUIRE(m->installWithDeps({"C"}));

  validateInstall(installPath, "A");
  validateInstall(installPath, "B");
  validateInstall(installPath, "C");

  m = new Manager(dataPath, installPath, ckVersion, langVersion,
		  "linux", X86_64, manifest_url_deps, false);

  INFO("Attempt to uninstall package A");
  // This should not succeed because B and C depend on A
  REQUIRE_FALSE(m->uninstall_with_deps("A", false));

  validateInstall(installPath, "A");

  m = new Manager(dataPath, installPath, ckVersion, langVersion, "linux",
                  X86_64, manifest_url_deps, false);

  INFO("Force uninstall package A");
  // This should not succeed because B and C depend on A
  REQUIRE(m->uninstall_with_deps("A", true));

  validateUninstall(installPath, "A");
}

// TEST need to upgrade a package
TEST_CASE("Dependency Tests - uninstall a pacakge") {
  TC::off();
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  INFO("Set up Manager");
  Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
                           "linux", X86_64, manifest_url_deps, false);


  // install package
  INFO("Installing package A");
  REQUIRE(m->installWithDeps({"A"}));

  validateInstall(installPath, "A");

  m = new Manager(dataPath, installPath, ckVersion, langVersion,
		  "linux", X86_64, manifest_url_deps, false);

  INFO("Attempt to uninstall package A");
  // This should not succeed because B and C depend on A
  REQUIRE(m->uninstall_with_deps("A", false));

  validateUninstall(installPath, "A");
}


TEST_CASE("Dependency Tests - will install auto update?") {
  TC::off();
  INFO("Test case start");
  fs::path installPath = makeTempDir();
  INFO("Temp Dir: " << installPath);

  std::string dataPath = "../data/manifest.json";

  ChuckVersion ckVersion = ChuckVersion("1.5.2.1");
  ApiVersion langVersion = ApiVersion("9.1");

  SECTION ("A") {
    INFO("Set up Manager");
    Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
			     "linux", X86_64, manifest_url_deps, false);


    // install package
    INFO("Installing package E (1.0.0)");
    REQUIRE(m->installWithDeps({"E=1.0.0"}));

    validateInstall(installPath, "E", "1.0.0");

    m = new Manager(dataPath, installPath, ckVersion, langVersion,
		    "linux", X86_64, manifest_url_deps, false);

    INFO("Should update E to 2.0.0");
    REQUIRE(m->installWithDeps({"E"}, true));

    validateInstall(installPath, "E", "2.0.0");
  }

  SECTION("B") {
    INFO("Set up Manager");
    Manager *m = new Manager(dataPath, installPath, ckVersion, langVersion,
			     "linux", X86_64, manifest_url_deps, false);


    // install package
    INFO("Installing package F (1.0.0)");
    REQUIRE(m->installWithDeps({"F=1.0.0"}));

    validateInstall(installPath, "F", "1.0.0");
    validateInstall(installPath, "E", "2.0.0");

    m = new Manager(dataPath, installPath, ckVersion, langVersion,
		    "linux", X86_64, manifest_url_deps, false);

    INFO("Upgrade F to 2.0.0");
    // This should not succeed because B and C depend on A
    REQUIRE(m->installWithDeps({"F"}, true));

    validateInstall(installPath, "F", "2.0.0");
    validateInstall(installPath, "E", "1.0.0");
  }
}
