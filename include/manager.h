// The manager interface

#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <string>
#include <vector>

#include "package.h"
#include "package_list.h"
#include "fetch.h"
#include "chuck_version.h"

using std::optional;
using std::string;
using std::tuple;

// only support the local path for now (to a spec json file)

class Manager {
 public:
  // constructor
  Manager(std::string package_list_path, fs::path package_install_dir, ChuckVersion ck_ver, ApiVersion api_ver, std::string system_os, std::string manifest_url, bool render_tui);

 public:
  // return a list of all packages (installed and available)
  std::vector<Package> listPackages();

  optional<Package> getPackage(string name);
  optional<PackageVersion> latestPackageVersion(string name);

  bool install(string packageName);
  bool uninstall(string packageName);
  bool update(string packageName);
  bool update_manifest(); // Download a new package manifest

  bool is_installed(Package pkg);

public:
  // the fetcher implementation
  FetchBase* fetch;

private:
  PackageList* package_list;
  ChuckVersion language_version;
  ApiVersion api_version;
  fs::path chump_dir;
  std::string os;
  std::string manifest_url;
};



#endif
