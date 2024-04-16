#include "uninstaller.h"
#include "util.h"

Uninstaller::Uninstaller(PackageList* pkg_list) {
  package_list = pkg_list;
}

bool Uninstaller::uninstall(std::string package_name) {
  // lookup package name (default to latest version)
  auto pkg = package_list->lookup(package_name);

  if (!pkg) {
    std::cerr << "BADBADBAD" << std::endl;
    return false;
  }

  auto package = pkg.value();

  fs::path package_dir = packagePath(package);

  // validate that it's actually a directory
  if (!fs::exists(package_dir) || !fs::is_directory(package_dir)) {
    std::cerr << "Package " << package.name << " was not found." << std::endl;
    return false;
  }

  // Delete entire directory - you've been warned
  fs::remove_all(package_dir);

  std::cout << "Package " << package.name << " was removed." << std::endl;

  return true;
}
