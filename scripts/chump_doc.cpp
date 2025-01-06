#include "package.h"
#include "package_directory.h"
#include "package_list.h"

#include <iostream>
#include <sstream>

namespace fs = std::filesystem;
using std::optional;
using std::string;

string package_doc(Package p);
string package_idx(PackageList p);

/*******************************************************************
 * Generates documentation webpages for each package
 *****************************************************************/

int main(int argc, const char **argv) {
  /*****************************************************************
   * CLI Args Setup
   *****************************************************************/
  // CLI::App app;

  // CLI11_PARSE(app, argc, argv);

  if (argc != 3) {
    std::cerr << "chump_doc must have two args: path to manifest directory and "
                 "output dir"
              << std::endl;
    return -1;
  }

  fs::path packages_path = argv[1];
  fs::path packages_subdir = packages_path / "packages";
  fs::path output_dir = argv[2];

  if (!fs::exists(packages_subdir) || !fs::is_directory(packages_subdir)) {
    std::cout << "no 'packages' dir found in " << packages_path
              << " make sure you pointed to the chump-packages repo correctly"
              << std::endl;
    return -1;
  }
  if (!fs::exists(output_dir)) {
    std::cout << "output dir '" << output_dir << "' not found" << std::endl;
    return -1;
  }

  std::vector<Package> packages;

  for (auto const &path : fs::directory_iterator{packages_subdir}) {
    if (!fs::is_directory(path))
      continue;

    // Each directory corresponds to a package

    // grab the package.json
    fs::path pkg_path = path.path() / "package.json";

    // std::cout << pkg_path << std::endl;

    if (!fs::exists(pkg_path)) {
      std::cerr
          << "Package definition " << pkg_path
          << "not found, are you in the chump-packages/packages directory?"
          << std::endl;
      continue;
    }

    Package pkg = read_package(pkg_path);

    populate_versions(&pkg, path);

    packages.push_back(pkg);
  }

  PackageList package_list(packages);

  fs::create_directory(output_dir / "packages");

  for (auto const &p : package_list.get_packages()) {
    fs::path filename = p.name + ".html";
    fs::path pkg_path = output_dir / "packages" / filename;
    std::ofstream out(pkg_path);
    out << package_doc(p);
    out.close();
  }

  fs::path idx_path = output_dir / "index.html";
  std::ofstream out(idx_path);
  out << package_idx(package_list);
  out.close();
}

string package_doc(Package p) {
  std::stringstream ss;

  ss << "<html>";
  ss << "<title>" << p.name << "</title>";
  ss << "<body>";

  ss << "<h1>" << p.name << "</h1>";

  ss << "<p> install command: chump install " << p.name << "</p>";

  // ss << "<p> authors: " << p.

  ss << "<p>" << p.description << "</p>";

  ss << "<p> Homepage: "
     << "<a href=\"" << p.homepage << "\">" << p.homepage << "</a>"
     << "</p>";
  ;
  ss << "<p> Repository: "
     << "<a href=\"" << p.repository << "\">" << p.repository << "</a>"
     << "</p>";

  ss << "<p> License: " << p.license << "</p>";

  // Current versions (mac, windows, linux)
  ss << "<p>"
     << "Current versions:"
     << "</p>";

  optional<PackageVersion> linux = p.latest_version("linux");
  optional<PackageVersion> win = p.latest_version("windows");
  optional<PackageVersion> mac = p.latest_version("mac");

  ss << "<table>";
  if (linux)
    ss << "<tr>"
       << "<th>"
       << "linux"
       << "</th>"
       << "<th>" << linux.value().getVersionString() << "</th>"
       << "</tr>";
  if (win)
    ss << "<tr>"
       << "<th>"
       << "windows"
       << "</th>"
       << "<th>" << win.value().getVersionString() << "</th>"
       << "</tr>";
  if (mac)
    ss << "<tr>"
       << "<th>"
       << "mac"
       << "</th>"
       << "<th>" << mac.value().getVersionString() << "</th>"
       << "</tr>";
  ss << "</table>";

  ss << "</body>";
  ss << "</html>";

  return ss.str();
}

string package_idx(PackageList pkg_list) {
  std::stringstream ss;

  ss << "<html>";
  ss << "<title>"
     << "ChuMP"
     << "</title>";
  ss << "<body>";

  ss << "<h1>"
     << "ChuMP (the ChucK Manager of Packages)"
     << "</h1>";
  ss << "<h2>"
     << "Package Directory"
     << "</h2>";

  ss << "<table>";
  for (auto const &p : pkg_list.get_packages()) {
    ss << "<tr>"
       << "<th>"
       << "<a href=\"./packages/" << p.name << ".html\">" << p.name << "</a>"
       << "</th>"
       << "<th>" << p.description << "</th>"
       << "</tr>";
  }
  ss << "</table>";

  return ss.str();
}
