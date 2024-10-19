#include "package_list.h"
#include "package.h"

// #include "CLI/CLI.hpp"

/*******************************************************************
   * Generates a complete manifest.json from the chuck-packages repo
   *****************************************************************/

#include <fstream>
#include <iostream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

Package read_package(fs::path filepath);
PackageVersion read_package_version(fs::path filepath);
void populate_versions(Package* p, fs::path pkg_path);

int main( int argc, const char ** argv ) {
  /*****************************************************************
   * CLI Args Setup
   *****************************************************************/
  // CLI::App app;

  // CLI11_PARSE(app, argc, argv);

  if (argc != 2) {
    std::cerr << "generate_manifest must have one arg: path to manifest directory" << std::endl;
    return -1;
  }

  // hard coded for now
  fs::path packages_path = argv[1];
  fs::path packages_subdir = packages_path / "packages";

  if (!fs::exists(packages_subdir) || !fs::is_directory(packages_subdir)) {
    std::cout << "no 'packages' dir found in " << packages_path
              << " make sure you pointed to the chump-packages repo correctly" << std::endl;
    return -1;
  }

  std::vector<Package> packages;

  for (auto const& path : fs::directory_iterator{packages_subdir}) {
    if (!fs::is_directory(path)) continue;

    // Each directory corresponds to a package

    // grab the package.json
    fs::path pkg_path = path.path() / "package.json";

    // std::cout << pkg_path << std::endl;

    if (!fs::exists(pkg_path)) {
      std::cerr << "Package definition " << pkg_path << "not found, are you in the chump-packages/packages directory?" << std::endl;
      continue;
    }

    Package pkg = read_package(pkg_path);

    populate_versions(&pkg, path);

    packages.push_back(pkg);
  }

  PackageList package_list(packages);

  json output = packages;
  std::cout << output.dump(4) << std::endl;

  return 0;
}

// traverse through a package directory and populate p with versions
void populate_versions(Package* p, fs::path pkg_path) {
  for (auto const& path : fs::directory_iterator{pkg_path}) {
    // std::cout << path << std::endl;

    if (fs::is_directory(path)) {
      populate_versions(p, path);
    } else if (path.path().extension() == ".json" && path.path().filename() != "package.json") {
      PackageVersion pkg_ver = read_package_version(path);
      p->versions.push_back(pkg_ver);
    }
  }
}

Package read_package(fs::path filepath) {
  std::ifstream f(filepath);

  if (!f.good()) {
    throw std::invalid_argument("Unable to open Package definition \"" + filepath.string() + "\"");
  }

  json data = json::parse(f);

  Package p = data.get<Package>();

  return p;
}

PackageVersion read_package_version(fs::path filepath) {
  std::ifstream f(filepath);

  if (!f.good()) {
    throw std::invalid_argument("Unable to open PackageVersion definition \"" + filepath.string() + "\"");
  }

  json data = json::parse(f);

  PackageVersion p = data.get<PackageVersion>();

  return p;
}
