#include "uninstaller.h"
#include "util.h"

Uninstaller::Uninstaller(PackageList* pkg_list, fs::path package_install_dir) {
  package_list = pkg_list;
  chump_dir = package_install_dir;
}

bool Uninstaller::uninstall(std::string package_name) {
  // lookup package name (default to latest version)
  auto pkg = package_list->find_package(package_name);

  if (!pkg) {
    std::cerr << "BADBADBAD" << std::endl;
    return false;
  }

  auto package = pkg.value();

  fs::path package_dir = packagePath(package, chump_dir);

  // Delete entire directory - you've been warned
  fs::remove_all(package_dir);

  std::cout << "Package " << package.name << " was removed." << std::endl;

  return true;
}
