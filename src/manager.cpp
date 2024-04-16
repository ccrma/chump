
#include "manager.h"
#include "util.h"

Manager::Manager() {
  fetch = new Fetch();
  package_list = new PackageList();
}

Manager::Manager(std::string package_list_path) {
  fetch = new Fetch();
  package_list = new PackageList(package_list_path);
  uninstaller = new Uninstaller(package_list);
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

  // TODO actually fetch operating system
  std::string os = whichOS();
  // fetch
  for (auto file: package.files[os]) {
    fetch->fetch(file, package);
  }

  // validate

  // return true (maybe find better return value)

  return true;
}

bool Manager::uninstall(std::string packageName) {
  if(!uninstaller->uninstall(packageName)) {
    std::cerr << "Failed to uninstall " << packageName << std::endl;
    return false;
  } else {
    std::cout << "Successfully uninstalled " << packageName << std::endl;
  }

  return true;
}
