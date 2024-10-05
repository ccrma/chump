// ChuMP (ChucK Manager of Programs) CLI main file.

//-----------------------------------------------------------------------------
// name: main()
// desc: command line chuck entry point
//-----------------------------------------------------------------------------

#include "chuck_version.h"
#include "exec.h"
#include "manager.h"
#include "util.h"

#include "CLI/CLI.hpp"

#include <filesystem>

namespace fs = std::filesystem;
using std::string;

int main( int argc, const char ** argv ) {

  /****************************************************************
   * CLI Args Setup
  /****************************************************************/
  CLI::App app;

  string usage = printLogo() + "\n" + "Usage: chump [OPTIONS] [SUBCOMMAND]";
  app.usage(usage);

  CLI::App* info = app.add_subcommand("info", "display information about <package>");
  string info_package_name;
  info
    ->add_option("package", info_package_name, "package to print info about")
    ->option_text("")
    ->required();

  CLI::App* list = app.add_subcommand("list", "list available packages");
  bool installed_flag = false;
  list->add_flag("-i, --installed", installed_flag, "list only installed packages");


  CLI::App* install = app.add_subcommand("install", "download and install <package>");
  string install_package_name;
  install
    ->add_option("package", install_package_name, "package to install (case sensitive)")
    ->option_text("(=<version>)")
    ->required();

  CLI::App* uninstall = app.add_subcommand("uninstall", "uninstall <package>");
  string uninstall_package_name;
  uninstall
    ->add_option("package", uninstall_package_name, "package to uninstall (case sensitive)")
    ->required();

  CLI::App* update = app.add_subcommand("update", "update <package> to latest compatible version");
  string update_package_name;
  update
    ->add_option("package", update_package_name, "package to update (case sensitive)")
    ->required();

  app.require_subcommand(1); // only want a single command here

  CLI11_PARSE(app, argc, argv);

  /****************************************************************
   * Chump setup
  /****************************************************************/

  fs::path path = chumpDir();

  fs::create_directories(path);

  Manager* manager;

  // Build manager and run command
  fs::path pkg_path = chumpDir() / "packages.json";
  try {
    manager = new Manager(pkg_path.string(), path, ChuckVersion::makeSystemVersion(), ApiVersion::makeSystemVersion(), whichOS(), true);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (app.got_subcommand(info)) {
    std::cout << info_package_name << std::endl;

    optional<Package> pkg = manager->getPackage(info_package_name);

    if (!pkg) {
      std::cerr << "Unable to find package " << info_package_name << std::endl;
      return 1;
    }

    std::cout << pkg.value() << std::endl;
  } else if (app.got_subcommand(list)) {
    printPackages(manager);
  } else if (app.got_subcommand(install)) {
    manager->install(install_package_name);
  } else if (app.got_subcommand(uninstall)) {
    manager->uninstall(uninstall_package_name);
  } else if (app.got_subcommand(update)) {
    manager->update(update_package_name);
  }

  return 0;
}
