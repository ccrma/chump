#ifndef __PACKAGELIST_H__
#define __PACKAGELIST_H__

#include <fstream>

#include <nlohmann/json.hpp>

#include "package.h"

using json = nlohmann::json;
using std::string;
using std::optional;


// A PackageList is a database of available packages, including names,
// download urls, description, and other metadata.
class PackageList {
 public:
  PackageList();
  PackageList(string filepath);

 public:
  // return latest (maybe latest && most compatible?) version of package
  optional<Package> lookup(string name);

  // return specific version of package
  optional<Package> lookup(string name, string version);


  /* static void from_json(const json& j, PackageList& pkg_list); */

 private:
  // TODO could be a more efficient lookup implementation
  std::vector<Package> packages;
};

#endif
