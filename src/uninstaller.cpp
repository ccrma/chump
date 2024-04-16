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

  std::string os = whichOS();

  // only delete files that chump knows about
  for (auto file: package.files[os]) {
    fs::path filename = fs::path(file).filename();

    fs::path filepath = package_dir / filename;

    if (!fs::remove(filepath)) {
      std::cerr << "File " << filepath << " not found." << std::endl;
    }
  }

  // If the package directory still has files, don't remove it.
  if (!fs::remove(package_dir)) {
    std::cerr << "Directory " << package_dir << " not removed." << std::endl;
  }

  return true;
}
