// The manager interface

#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <string>
#include <vector>

#include "package.h"
#include "package_list.h"
#include "fetch.h"
#include "uninstaller.h"

using std::optional;
using std::string;

// only support the local path for now (to a spec json file)

class Manager {
 public:
  // constructor
  Manager();
  Manager(std::string package_list_path);

 public:
  // return a list of all packages (installed and available)
  std::vector<Package> listPackages();

  optional<Package> getPackage(string name);

  bool install(string packageName);
  bool uninstall(string packageName);
  bool update(string packageName); // TODO update install specific version


public:
  // the fetcher implementation
  FetchBase* fetch;

private:
  PackageList* package_list;
  UninstallerBase* uninstaller;
};



#endif
