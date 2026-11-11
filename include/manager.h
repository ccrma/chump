// The manager interface

#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <repo.h>
#include <solver.h>
#include <string>
#include <vector>

#include "chuck_version.h"
#include "fetch.h"
#include "generator.hpp"
#include "package.h"
#include "package_list.h"

using std::optional;
using std::string;
using std::tuple;
namespace fs = std::filesystem;

// defines a step in the installation/uninstallation/upgrade process
enum step_type { NONE, INSTALL, UNINSTALL, UPGRADE };

struct install_result {
  install_result(string _name, bool _finished = false, bool _success = false)
      : name(_name), success(_success), finished(_finished), step(NONE),
        cont(false) {}

  string name;
  bool success;
  bool finished;

  step_type step;

  // If all installs are able to download, continue to installing them in
  // install_dir
  bool cont;
};

// install_with_deps_result is any action that needs to handled by the user.
// because installations can result in install, uninstall, and upgrade/downgrade
// actions, we need to classify which one is being taken.
//
// action_type is which action is being done
// finished indicateds the coroutine is done
// success indicates whether the install_with_deps call failed or not
struct install_with_deps_result {
  install_with_deps_result(string _name, bool _finished = false,
                           bool _success = false)
      : name(_name), finished(_finished), success(_success), cont(false) {}

  string name;
  step_type action_type;
  bool finished;
  bool success;
  bool cont;

  string old_version;
  string new_version;
};

class Manager {
public:
  // constructor
  Manager(std::string package_list_path, fs::path package_install_dir,
          ChuckVersion ck_ver, ApiVersion api_ver, std::string system_os,
          Architecture arch, std::string manifest_url, bool render_tui);

public:
  // return a list of all packages (in manifest or installed)
  std::vector<Package> listPackages();
  std::vector<InstalledVersion> listInstalledVersions();

  optional<Package> getPackage(string name);
  optional<PackageVersion> latestPackageVersion(string name);

  // fetch a package to a tmp dir (this is the pre-install step)
  optional<fs::path> fetch_to_tmp(string packageName, bool isDep = false);
  // install a package from a tmp dir to the install dir
  bool install(string packageName, fs::path tmp_dir);
  Generator<install_with_deps_result *>
  install_with_deps(const vector<string> packageNames, bool update = false);
  bool install_local(fs::path pkgDefn, fs::path pkgVer, fs::path pkgZip);
  bool install_local_with_deps(fs::path pkgDefn, fs::path pkgVer,
                               fs::path pkgZip);
  bool uninstall_with_deps(string packageName, bool force);
  bool uninstall(string packageName, bool force);
  bool update_manifest();                 // Download a new package manifest
  bool open_doc(string packageName);      // open doc page
  bool open_examples(string packageName); // open doc page

  bool is_installed(Package pkg);
  fs::path install_path(Package pkg);

  // returns true if packageName is:
  // 1. installed and
  // 2. is a user-installed package
  // return false otherwise
  bool is_user_installed(string packageName);

  string getSystemChuckVersion();

public:
  // the fetcher implementation
  FetchBase *fetch;

private:
  PackageList *package_list;
  ChuckVersion language_version;
  ApiVersion api_version;
  fs::path chump_dir;
  std::string os;
  Architecture arch;
  std::string manifest_url;

public:
  // attempts to open json files
  optional<Package> open_package_file(fs::path path);
  optional<InstalledVersion> open_installed_version_file(fs::path path);
  optional<PackageVersion> open_package_version_file(fs::path path);

private: // dependency resolution variables
  Repo *initializeSystemRepo();
  Repo *initializeManifestRepo();
  Pool *pool;
  Repo *systemRepo;
  Repo *manifestRepo;

  // if upgrading, need to extract what version p should be upgraded
  // to from the transaction
  string getNewVersion(Transaction *trans, Id p);
};

#endif
