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
  // check if we output to TTY (teletype interactive terminal)
  // if not disable color teriminal to avoid outputing
  // ANSI escape codes to the output stream, which would
  // show up in | and >
  t_CKBOOL colorTerminal = ck_isatty();
  // set the color terminal global setting
  TC::globalDisableOverride( !colorTerminal );

  /****************************************************************
   * CLI Args Setup
   ****************************************************************/
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
  auto pkg_opt = update
    ->add_option("package", update_package_name, "package to update (case sensitive)");
    // ->required();
  bool update_package_list = false;
  auto update_list_flag = update->add_flag("-u, --update-list", update_package_list, "update list of available packages");

  update_list_flag->excludes(pkg_opt);

  CLI::App* cereal = app.add_subcommand("cereal", "a fruity breakfast treat");
  CLI::App* river = app.add_subcommand("river", "flowing & ebbing");

  app.require_subcommand(1); // only want a single command here

  CLI11_PARSE(app, argc, argv);

  /****************************************************************
   * Chump setup
   ****************************************************************/

  fs::path path = chumpDir();

  fs::create_directories(path);

  Manager* manager;

  // Build manager and run command
  fs::path pkg_path = chumpDir() / "manifest.json";

  std::string manifest_url = "https://ccrma.stanford.edu/~nshaheed/chump/manifest.json";

  // if the manifesto isn't loading properly, only allow `chump update -u`.
  // This is an escape hatch, because failing to parse manifest.json will
  // result in an exception and the program won't continue.
  if (!validate_manifest(pkg_path)) {
    if (!update_package_list) return -1; // exit if we're not updating the manifest

    manager = new Manager("", path, ChuckVersion::makeSystemVersion(),
                          ApiVersion::makeSystemVersion(), whichOS(), manifest_url, true);

    manager->update_manifest();

    return 1;
  }


  try {
    manager = new Manager(pkg_path.string(), path, ChuckVersion::makeSystemVersion(),
                          ApiVersion::makeSystemVersion(), whichOS(), manifest_url, true);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (app.got_subcommand(info)) {
    optional<Package> pkg = manager->getPackage(info_package_name);

    if (!pkg) {
      std::cerr << "Unable to find package " << info_package_name << std::endl;
      return 1;
    }

    std::cout << pkg.value() << std::endl;
  } else if (app.got_subcommand(list)) {
    printPackages(manager, installed_flag);
  } else if (app.got_subcommand(install)) {
    manager->install(install_package_name);
  } else if (app.got_subcommand(uninstall)) {
    manager->uninstall(uninstall_package_name);
  } else if (app.got_subcommand(update)) {
    if (update_package_list) {
      manager->update_manifest();
    } else {
      manager->update(update_package_name);
    }
  } else if (app.got_subcommand(cereal)) {
    int counter = 0;
    while (true) {

      std::string logo = jumbleLogo(counter);
      std::cout << logo;

      auto num_lines = std::count( logo.begin(), logo.end(), '\n' );

      for (int i = 0; i < num_lines; i++) {
        std::cout << "\x1b[F"; // Move the cursor up one line
      }
      counter++;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  } else if (app.got_subcommand(river)) {
    int counter = 0;
    while (true) {

      std::string logo = riverLogo(counter);
      std::cout << logo;

      auto num_lines = std::count( logo.begin(), logo.end(), '\n' );

      for (int i = 0; i < num_lines; i++) {
        std::cout << "\x1b[F"; // Move the cursor up one line
      }
      counter++;
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  return 0;
}
