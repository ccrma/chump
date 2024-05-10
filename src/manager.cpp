
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

  // if there is already a .chump/PACKAGE directory, error out and tell the user to call update
  fs::path install_dir = packagePath(package);

  if (fs::exists(install_dir)) {
    std::cerr << "The install directory '" << install_dir << "' already exists." << std::endl;
    std::cerr << "Use `chump update " << package.name << "' to update the existing package" << std::endl;
    std::cerr << "Or use `chump uninstall " << package.name << "` to remove the package" << std::endl;
    return false;
  }

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
    bool result = fetch->fetch(file, package);
    if (!result) {
      std::cerr << "Failed to fetch " << file << ", exiting." << std::endl;
      return false;
    }
  }

  // Write version.json to file.
  json version_json = version;

  std::ofstream o(install_dir / "version.json");
  o << std::setw(4) << version_json << std::endl;

  // validate

  // return true (maybe find better return value)

  return true;
}

bool Manager::update(string packageName) {
  // lookup package name (default to latest version)
  auto pkg = package_list->find_package(packageName);

  if (!pkg) {
    std::cerr << "Package " << packageName << " not found." << std::endl;
    return false;
  }

  std::cout << "update!\n";

  Package package = pkg.value();

  // if there is already a .chump/PACKAGE directory, error out and tell the user to call update
  fs::path install_dir = packagePath(package);

  if (!fs::exists(install_dir)) {
    std::cerr << "The install directory '" << install_dir << "' does not exist." << std::endl;
    std::cerr << "Use `chump install " << package.name << "' to install the existing package" << std::endl;
    return false;
  }

  // TODO validate that the version is newer

  std::string os = whichOS();

  optional<PackageVersion> ver = package.latest_version(os);

  if (!ver) {
    std::cerr << "Unable to find version of package " << package.name
              << " that works on your system" << std::endl;

    return false;
  }

  PackageVersion version = ver.value();

  // remove old package
  // std::cout << install_dir << std::endl;
  fs::remove_all(install_dir);

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
