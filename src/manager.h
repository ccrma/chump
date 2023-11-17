// The manager interface

#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <string>
#include <vector>

#include "package.h"
#include "fetch.h"

// only support the local path for now (to a spec json file)

class Manager {
 public:
  // constructor
  Manager();

 public:
  // return a list of all packages (installed and available)
  std::vector<Package> listPackages();

  Package getPackage(std::string name);


public:
  // the fetcher implementation
  FetchBase* fetch;
};

#endif
