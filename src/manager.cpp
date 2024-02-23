
#include "manager.h"

Manager::Manager() {
  fetch = new Fetch();
  package_list = new PackageList();
}

optional<Package> Manager::getPackage(string name) {
  return fetch->fetch(name);
}

bool Manager::install(std::string packageName) {
  // lookup package name (default to latest version)
  auto package = package_list->lookup(packageName);

  if (!package) {
    std::cerr << "BADBADBAD" << std::endl;
  }

  // fetch


  // validate

  // return true (maybe find better return value)
}


