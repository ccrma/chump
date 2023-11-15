// The manager interface

#include "package.h"
#include <string>
#include <vector>

// only support the local path for now (to a spec json file)

class Manager {
 public:
  // constructor
  Manager();

 public:
  // return a list of all packages (installed and available)
  std::vector<Package> listPackages();

  Package getPackage(std::string name);


private:
  // this is loaded from *somewhere* an initialization
  std::vector<Package> packages;
  
};
