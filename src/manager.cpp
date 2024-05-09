
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

optional<Package> Manager::getPackage(string packageName) {
  return package_list->find_package(packageName);
}

bool Manager::install(std::string packageName) {
  // lookup package name (default to latest version)
  auto pkg = package_list->find_package(packageName);

  if (!pkg) {
    std::cerr << "Package " << packageName << " not found." << std::endl;
    return false;
  }

  Package package = pkg.value();

  std::string os = whichOS();

  optional<PackageVersion> ver = package.latest_version(os);

  if (!ver) {
    std::cerr << "Unable to find version of package " << package.name
              << " that works on your system" << std::endl;

    return false;
  }

  PackageVersion version = ver.value();

  // fetch
  for (auto file: version.files) {
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

std::vector<Package> Manager::listPackages() {
  return package_list->get_packages();
}
