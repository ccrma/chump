
#include "package_list.h"


PackageList::PackageList() {
  throw std::runtime_error("not implemented");
}

PackageList::PackageList(string filepath) {
  std::ifstream f(filepath);

  // TODO better error checks
  if (!f.good()) {
    std::cout << "BADBADBAD" << std::endl;
  }
  
  json data = json::parse(f);

  // parse all the packages
  json j_packages = data["packages"];

  for (auto& j_package : j_packages) {
    std::cout << j_package << '\n';
    Package p = j_package.get<Package>();

    packages.push_back(p);
  }
}

optional<Package> PackageList::lookup(string name) {
  for (auto package: packages) {
    if (package.name == name) {
      return package;
    }
  }
  return {};
}

optional<Package> PackageList::lookup(string name, string version) {
  throw std::runtime_error("not implemented");
}

