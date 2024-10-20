
#include "manager.h"
#include "util.h"

#include <regex>

Manager::Manager(string package_list_path, fs::path package_install_dir, ChuckVersion ck_ver, ApiVersion api_ver, string system_os, string _manifest_url, bool render_tui) {
  chump_dir = package_install_dir;
  os = system_os;
  manifest_url = _manifest_url;

  fetch = new Fetch(render_tui);

  // really only used to update manifest without needed to properly
  // parse the existing manifest.
  if (package_list_path == "") package_list = new PackageList();
  else package_list = new PackageList(package_list_path);
  uninstaller = new Uninstaller(package_list, chump_dir);

  language_version = ck_ver;
  api_version = api_ver;
}

optional<Package> Manager::getPackage(string packageName) {
  return package_list->find_package(packageName);
}

optional<PackageVersion> Manager::latestPackageVersion(string name) {
  return package_list->find_latest_package_version(name, os, language_version, api_version);
}

bool Manager::install(string packageName) {
  // parse packages here
  auto [name, version_string] = parsePackageName(packageName);

  // lookup package name (default to latest version)
  auto pkg = package_list->find_package(name);

  if (!pkg) {
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

  if (fs::exists(install_dir / "version.json")) {
    std::cerr << "The package '" << package.name << "' already exists." << std::endl;
    std::cerr << "Use `chump update " << package.name << "' to update the existing package" << std::endl;
    std::cerr << "Or use `chump uninstall " << package.name << "` to remove the package" << std::endl;
    return false;
  }

  // Create a temporary directory to download our files to
  fs::path temp_dir = {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directory(temp_dir);

  // fetch
  for (auto file: version.files) {
    fs::path dir = file.local_dir;
    string url = file.url;
    string checksum = file.checksum;

    bool result = fetch->fetch(url, dir, package, temp_dir, checksum);
    if (!result) {
      std::cerr << "Failed to fetch " << url << ", exiting." << std::endl;
      return false;
    }
  }

  // create install dir if needed
  fs::create_directory(install_dir);

  // Copy temp files over to the install directory
  try {
    fs::copy(temp_dir, install_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
  } catch (std::filesystem::filesystem_error& e) {
    std::cerr << e.what() << '\n';
    return false;
  }

  // Removing temp dir
  fs::remove_all(temp_dir);

  // Write version.json to file.
  json version_json = version;

  std::ofstream o(install_dir / "version.json");
  o << std::setw(4) << version_json << std::endl;
  o.close();

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
  f.close();
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

  for (auto file: installed_version.files) {
    fs::path dir = file.local_dir;
    string url = file.url;
    fs::path filename = fs::path(url).filename();

    fs::remove(install_dir / dir / filename);
    if (fs::is_empty(install_dir / dir)) {
      fs::remove(install_dir / dir);
    }
  }


  // Create a temporary directory to download our files to
  fs::path temp_dir = {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directory(temp_dir);

  // fetch
  for (auto file: latest_version.files) {
    fs::path dir = file.local_dir;
    string url = file.url;
    string checksum = file.checksum;

    bool result = fetch->fetch(url, dir, package, temp_dir, checksum);
    if (!result) {
      std::cerr << "Failed to fetch " << url << ", exiting." << std::endl;
      return false;
    }
  }

  // create install dir if needed
  fs::create_directory(install_dir);

  // Copy temp files over to the install directory
  try {
    fs::copy(temp_dir, install_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
  } catch (fs::filesystem_error& e) {
    std::cerr << e.what() << '\n';
    return false;
  }

  // Write version.json to file.
  json latest_version_json = latest_version;

  std::ofstream o(install_dir / "version.json");
  o << std::setw(4) << latest_version_json << std::endl;
  o.close();

  return true;
}

bool Manager::uninstall(string packageName) {
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
  f.close();
  PackageVersion curr_ver = pkg_ver.template get<PackageVersion>();

  optional<PackageVersion> installed_version = package_list->find_package_version(packageName, curr_ver);

  if (!installed_version) {
    std::cerr << "Unable to find package version" << std::endl;
    return false;
  }

  // Remove all files associated with package
  for (auto file: installed_version.value().files) {
    fs::path dir = file.local_dir;
    string url = file.url;

    fs::path filename = fs::path(url).filename();

    fs::remove(install_dir / dir / filename);
    if (fs::is_empty(install_dir / dir)) {
      fs::remove(install_dir / dir);
    }
  }

  fs::remove(install_dir / "version.json"); // This is generated by chump

  // Clean up the directory if it's empty. If there's stuff there that
  // was added not by the installer, treat that as not-touched so that
  // if you reinstall the package your files will still be there.
  if (fs::is_empty(install_dir)) {
    fs::remove(install_dir);
  }

  return true;
}

std::vector<Package> Manager::listPackages() {
  return package_list->get_packages();
}

bool Manager::is_installed(Package pkg) {
  fs::path install_dir = packagePath(pkg, chump_dir);

  return fs::exists(install_dir / "version.json");
}

bool Manager::update_manifest() {
  // Create a temporary directory to download our manifest to
  fs::path temp_dir = {fs::temp_directory_path() /= std::tmpnam(nullptr)};
  fs::create_directory(temp_dir);

  // load both tmp file and current file as strings
  bool result = fetch->fetch_manifest(manifest_url, temp_dir);

  if (!result) {
    std::cerr << "Failed to fetch manifest.json, exiting." << std::endl;
    return false;
  }

  // check if different (.compare). If they are, copy over the new file
  std::ifstream temp_manifest(temp_dir / "manifest.json");
  std::ifstream curr_manifest(chump_dir / "manifest.json");

  // Compare the two files. If they're different, copy over the newly-fetched
  // manifest.
  // https://stackoverflow.com/questions/15118661/whats-the-fastest-way-to-tell-whether-two-strings-or-binary-files-are-different
  std::istreambuf_iterator<char> temp_manifest_iter(temp_manifest);
  std::istreambuf_iterator<char> curr_manifest_iter(curr_manifest);

  bool are_equal = true;

  std::istreambuf_iterator<char> end;

  while(temp_manifest_iter != end && curr_manifest_iter != end) {
    if(*temp_manifest_iter != *curr_manifest_iter) {
      are_equal = false;
      break;
    }
    ++temp_manifest_iter;
    ++curr_manifest_iter;
  }

  if ((temp_manifest_iter != end) || (curr_manifest_iter == end)) {
    are_equal = false;
  }

  if (are_equal) {
    std::cerr << "current manifest is up-to-date, doing nothing" << std::endl;
    return false;
  }

  // Copy temp files over to the install directory
  try {
    fs::remove(chump_dir / "manifest.json");
    fs::copy(temp_dir / "manifest.json", chump_dir / "manifest.json",
             fs::copy_options::overwrite_existing);
  } catch (std::filesystem::filesystem_error& e) {
    std::cerr << e.what() << '\n';
    return false;
  }

  std::cerr << "manifest.json was successfully updated!" << std::endl;
  return true;
}
