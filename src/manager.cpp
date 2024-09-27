
#include "manager.h"
#include "util.h"

#include <regex>

Manager::Manager(string package_list_path, fs::path package_install_dir, ChuckVersion ck_ver, ApiVersion api_ver, string system_os) {
  chump_dir = package_install_dir;
  os = system_os;

  fetch = new Fetch;
  package_list = new PackageList(package_list_path, os, ck_ver, api_ver);
  uninstaller = new Uninstaller(package_list, chump_dir);

  language_version = ck_ver;
  api_version = api_ver;
}

optional<Package> Manager::getPackage(string packageName) {
  return package_list->find_package(packageName);
}

optional<PackageVersion> Manager::latestPackageVersion(string name) {
  return package_list->find_latest_package_version(name);
}

bool Manager::install(string packageName) {
  // parse packages here
  auto [name, version_string] = parsePackageName(packageName);

  // lookup package name (default to latest version)
  auto pkg = package_list->find_package(name);

  if (!package_list->find_package(name)) {
    std::cerr << "Package " << packageName << " not found." << std::endl;
    return false;
  }

  Package package = pkg.value();

  optional<PackageVersion> ver;
  if (version_string) {
    try {
      PackageVersion pkgver(version_string.value());
      ver = package.version(pkgver, os, language_version, api_version);
    } catch (std::invalid_argument& e) {
      std::cerr << e.what() << '\n';
      return false;
    }
  } else {
    ver = package.latest_version(os, language_version, api_version);
  }

  if (!ver) {
    std::cerr << "Unable to find version of package " << package.name
              << " that works on your system" << std::endl;

    return false;
  }

  PackageVersion version = ver.value();

  // if there is already a .chump/PACKAGE directory, error out and tell the user to call update
  fs::path install_dir = packagePath(package, chump_dir);

  if (fs::exists(install_dir)) {
    std::cerr << "The install directory '" << install_dir << "' already exists." << std::endl;
    std::cerr << "Use `chump update " << package.name << "' to update the existing package" << std::endl;
    std::cerr << "Or use `chump uninstall " << package.name << "` to remove the package" << std::endl;
    return false;
  }

  // Create a temporary directory to download our files to
  fs::path temp_dir = {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directory(temp_dir);

  // fetch
  for (auto file: version.files) {
    bool result = fetch->fetch(file, package, temp_dir);
    if (!result) {
      std::cerr << "Failed to fetch " << file << ", exiting." << std::endl;
      return false;
    }
  }

  // create install dir if needed
  fs::create_directory(install_dir);

  // Copy temp files over to the install directory
  try {
    fs::copy(temp_dir, install_dir, std::filesystem::copy_options::recursive);
  } catch (std::filesystem::filesystem_error& e) {
    std::cerr << e.what() << '\n';
    return false;
  }

  // Write version.json to file.
  json version_json = version;

  std::ofstream o(install_dir / "version.json");
  o << std::setw(4) << version_json << std::endl;

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

  Package package = pkg.value();

  // if there is already a .chump/PACKAGE directory, error out and tell the user to call update
  fs::path install_dir = packagePath(package, chump_dir);

  if (!fs::exists(install_dir)) {
    std::cerr << "The install directory '" << install_dir << "' does not exist." << std::endl;
    std::cerr << "Use `chump install " << package.name << "' to install the existing package" << std::endl;
    return false;
  }

  // Validate that the version is newer
  std::ifstream f(install_dir / "version.json");

  if (!f.good()) {
    std::cerr << "Unable to open " << install_dir / "version.json" << std::endl;
    return false;
  }

  json pkg_ver = json::parse(f);
  PackageVersion installed_version = pkg_ver.template get<PackageVersion>();

  optional<PackageVersion> ver = package.latest_version(os, language_version, api_version);

  if (!ver) {
    std::cerr << "Unable to find version of package " << package.name
              << " that works on your system" << std::endl;

    return false;
  }

  PackageVersion latest_version = ver.value();

  if (installed_version == latest_version) {
    std::cout << package.name << " is already up-to-date." << std::endl;
    return true;
  }

  if (installed_version > latest_version) {
    std::cout << package.name << " is installed already with a newer version." << std::endl;
    return true;
  }

  // remove old package
  fs::remove_all(install_dir);

  // Create a temporary directory to download our files to
  fs::path temp_dir = {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directory(temp_dir);

  // fetch
  for (auto file: latest_version.files) {
    bool result = fetch->fetch(file, package, temp_dir);
    if (!result) {
      std::cerr << "Failed to fetch " << file << ", exiting." << std::endl;
      return false;
    }
  }

  // create install dir if needed
  fs::create_directory(install_dir);

  // Copy temp files over to the install directory
  try {
    std::filesystem::copy(temp_dir, install_dir, std::filesystem::copy_options::recursive);
  } catch (std::filesystem::filesystem_error& e) {
    std::cerr << e.what() << '\n';
    return false;
  }

  // Write version.json to file.
  json latest_version_json = latest_version;

  std::ofstream o(install_dir / "version.json");
  o << std::setw(4) << latest_version_json << std::endl;

  return true;
}

bool Manager::uninstall(string packageName) {
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
