#ifndef __PACKAGELIST_H__
#define __PACKAGELIST_H__

#include <fstream>
#include <optional>
#include <nlohmann/json.hpp>

#include "package.h"
#include "util.h"
// #include "chuck_version.h"

using json = nlohmann::json;
using std::string;
using std::optional;


// A PackageList is a database of available packages, including names,
// download urls, description, and other metadata.
class PackageList {
 public:
  PackageList();
  PackageList(string filepath) : PackageList(filepath, whichOS(),
                                             ChuckVersion::makeSystemVersion(),
                                             ApiVersion::makeSystemVersion()
                                             ) {};
  // set OS explicitly for unit tests
  PackageList(string filepath, string operating_system, ChuckVersion ck_ver, ApiVersion api_ver);

 public:
  optional<Package> find_package(string name);
  // return latest (maybe latest && most compatible?) version of package
  optional<PackageVersion> find_latest_package_version(string name);

  // return specific version of package
  optional<PackageVersion> find_package_version(string name, string version);
  optional<PackageVersion> find_package_version(string name, PackageVersion version);
  std::vector<Package> get_packages();


  /* static void from_json(const json& j, PackageList& pkg_list); */

 private:
  // TODO could be a more efficient lookup implementation
  std::vector<Package> packages;

  ChuckVersion language_version;
  ApiVersion api_version;

  // set OS at initialization time
  string os;
};

#endif
