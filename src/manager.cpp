
#include "manager.h"

Manager::Manager() {
  fetch = new Fetch();
  package_list = new PackageList();
}

Manager::Manager(std::string package_list_path) {
  fetch = new Fetch();
  package_list = new PackageList(package_list_path);
}

optional<Package> Manager::getPackage(string name) {
  // return fetch->fetch(name);
  return {};
}

bool Manager::install(std::string packageName) {
  // lookup package name (default to latest version)
  auto pkg = package_list->lookup(packageName);

  if (!pkg) {
    std::cerr << "BADBADBAD" << std::endl;
    return false;
  }

  auto package = pkg.value();

  // fetch
  for (auto file: package.files) {
    fetch->fetch(file, package.name);
  }

  // validate

  // return true (maybe find better return value)

  return true;
}


