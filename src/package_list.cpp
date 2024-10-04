
#include "package_list.h"


PackageList::PackageList() {
  // throw std::runtime_error("not implemented");
}

PackageList::PackageList(string filepath, string operating_system,
                         ChuckVersion ck_ver, ApiVersion api_ver) {
  os = operating_system;
  language_version = ck_ver;
  api_version = api_ver;

  std::ifstream f(filepath);

  // TODO better error checks
  if (!f.good()) {
    throw std::invalid_argument("Unable to open Package List \"" + filepath + "\"");
  }

  json data = json::parse(f);

  // parse all the packages
  json j_packages = data["packages"];

  for (auto& j_package : j_packages) {
    Package p = j_package.get<Package>();

    packages.push_back(p);
  }

  // Get language and api version
}

optional<Package> PackageList::find_package(string name) {
  for (auto package: packages) {
    if (package.name == name) {
      return package;
    }
  }

  return {};
}

optional<PackageVersion> PackageList::find_latest_package_version(string name) {
  // TODO get highest version (how do I do that?)
  for (auto package: packages) {
    if (package.name == name) {
      return package.latest_version(os, language_version, api_version);
    }
  }
  return {};
}

optional<PackageVersion> PackageList::find_package_version(string name, string version) {
  PackageVersion want(version);

  for (auto package: packages) {
    if (package.name == name) {
      for (auto package_version: package.versions) {
        if (package_version == want) {
          return package_version;
        }
      }
    }
  }
  return {};
}

optional<PackageVersion> PackageList::find_package_version(string name, PackageVersion version) {
  for (auto package: packages) {
    if (package.name == name) {
      for (auto package_version: package.versions) {
        if (package_version == version) {
          return package_version;
        }
      }
    }
  }
  return {};
}

std::vector<Package> PackageList::get_packages() {
  return packages;
}
