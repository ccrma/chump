
#include "manager.h"
#include "util.h"

#include <pool.h>
#include <regex>
#include <solver.h>
#include <sstream>
#include <unordered_set>

void printDependencies(Pool *pool, const char *packageName);

Manager::Manager(string package_list_path, fs::path package_install_dir,
                 ChuckVersion ck_ver, ApiVersion api_ver, string system_os,
                 Architecture _arch, string _manifest_url, bool render_tui) {
  chump_dir = package_install_dir;
  os = system_os;
  arch = _arch;
  manifest_url = _manifest_url;

  fetch = new Fetch(render_tui);

  language_version = ck_ver;
  api_version = api_ver;

  // always try to update manifest
  update_manifest();

  // really only used to update manifest without needed to properly
  // parse the existing manifest.
  package_list = new PackageList(package_list_path);

  pool = pool_create();
  systemRepo = initializeSystemRepo();
  manifestRepo = initializeManifestRepo();

  // testSolve();
}

optional<Package> Manager::getPackage(string packageName) {
  return package_list->find_package(packageName);
}

optional<PackageVersion> Manager::latestPackageVersion(string name) {
  return package_list->find_latest_package_version(
      name, os, arch, language_version, api_version);
}

//-----------------------------------------------------------------------------
// name: installWithDeps()
// desc: Install, but with dependencies!
//       Uses libsolv to resolve package dependencies via a sat solver
//-----------------------------------------------------------------------------
bool Manager::installWithDeps(const vector<string> packageNames) {
  // make install list
  Queue job;
  queue_init(&job);

  std::unordered_set<Id> requestedPackages;

  for (auto packageName : packageNames) {
    // parse packages here
    auto [name, version_string] = parsePackageName(packageName);

    // lookup package name (default to latest version)
    auto pkg = package_list->find_package(name);

    if (!pkg) {
      std::cerr << chumpPrepend() << "package " << packageName << " not found."
                << std::endl;
      return false;
    }

    // printDependencies(pool, name.c_str());

    // define job
    Id name_id = pool_str2id(pool, name.c_str(), 1);

    requestedPackages.insert(name_id);

    // If a package version is specified, have both the name
    // and version as solver constraints, otherwise, just
    // constrain on the name.
    if (version_string) {
      Id evr_id = pool_str2id(pool, version_string.value().c_str(), 1);

      // create a relational ID (i.e. "Rec = 1.1.0")
      Id rel_id = pool_rel2id(pool, name_id, evr_id, REL_EQ, 1);
      queue_push2(&job, SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES, rel_id);
    } else {
      queue_push2(&job, SOLVER_INSTALL | SOLVER_SOLVABLE_NAME, name_id);
    }
  }

  // std::cout << "--- libsolv Pool State Dump ---\n";
  // // Solvables are stored in a contiguous array from index 1 to
  // // pool->nsolvables
  //
  // for (int i = 1; i < pool->nsolvables; i++) {
  //   Solvable *s = pool->solvables + i;

  //   // Only print actual packages, skip empty slots
  //   if (s->repo) {
  //     std::cout << "Repo: " << s->repo->name
  // 		<< " | Pkg: '" << pool_id2str(pool, s->name) << "'"
  // 		<< " ver '" << pool_id2str(pool, s->evr) << "'"
  // 		<< " (Name ID: " << s->name << ")\n";
  //   }
  // }
  // std::cout << "-------------------------------\n";

  // Create and run solver
  Solver *solver = solver_create(pool);

  // Allow the solver to downgrade installed packages if a dependency requires
  // it
  solver_set_flag(solver, SOLVER_FLAG_ALLOW_DOWNGRADE, 1);

  // Solve it!
  solver_solve(solver, &job);

  // check for solver success
  int problem_count = solver_problem_count(solver);
  if (problem_count > 0) {
    std::cerr << chumpPrepend() << "dependency resolution failed with "
              << problem_count << " problems:\n";

    Id problem = 0;
    // Pass 0 to get the first problem, then pass the previous ID to get the
    // next
    while ((problem = solver_next_problem(solver, problem)) != 0) {
      std::cerr << " - " << solver_problem2str(solver, problem) << "\n";
    }

    return false;
  }

  // parse the transaction and install/erase/upgrade
  // there are a lot of options:
  // https://github.com/openSUSE/libsolv/blob/c9ee7a3b946e8870f056dddc22032e4d43a40636/src/transaction.h#L45
  Transaction *trans = solver_create_transaction(solver);

  if (trans->steps.count == 0) {
    std::cerr << chumpPrepend()
              << "the requested packages are already installed." << std::endl;
    std::cerr << chumpPrepend()
              << "use `chump update <package>' to update the existing "
                 "package"
              << std::endl;
    std::cerr << chumpPrepend()
              << "or use `chump uninstall <package>` to remove the package"
              << std::endl;

    transaction_free(trans);
    solver_free(solver);
    queue_free(&job);
    return true; // Success, no file operations needed
  }

  // topologically order transactions
  transaction_order(trans, 0);

  for (int i = 0; i < trans->steps.count; ++i) {
    Id p = trans->steps.elements[i];
    Solvable *s = pool_id2solvable(pool, p);

    // Determine what action the solver is actually taking on this package
    // Id step_type = transaction_type(trans, p, SOLVER_TRANSACTION_SHOW_ALL);
    Id step_type = transaction_type(trans, p, 0);

    // ignore ck & api version (we can't install these!)
    if (strcmp(pool_id2str(pool, s->name), "chuck-system-version") == 0)
      continue;
    if (strcmp(pool_id2str(pool, s->name), "chuck-system-api") == 0)
      continue;

    if (step_type == SOLVER_TRANSACTION_INSTALL) {
      string s_name = pool_id2str(pool, s->name);
      string s_evr = pool_id2str(pool, s->evr);

      bool is_requested = requestedPackages.count(s->name) > 0;

      // Print different message depending on if it's dependency or not
      if (is_requested) {
        std::cout << chumpPrepend()
                  << "installing package: " << packageStr(s_name, s_evr)
                  << "\n";
      } else {
        std::cout << chumpPrepend()
                  << "installing dependency: " << packageStr(s_name, s_evr)
                  << "\n";
      }

      string pkgstr = string(pool_id2str(pool, s->name)) + "=" +
                      string(pool_id2str(pool, s->evr));

      if (!install(pkgstr)) {
        std::cerr << "Failed to install\n";
        return false;
      }
    } else if (step_type == SOLVER_TRANSACTION_ERASE) {
      std::cout << chumpPrepend()
                << "removing package: " << pool_id2str(pool, s->name) << "="
                << pool_id2str(pool, s->evr) << "\n";

      string pkg = string(pool_id2str(pool, s->name));
      string pkgstr = pkg + "=" + string(pool_id2str(pool, s->evr));

      if (!uninstall(pkg, false)) {
        std::cerr << chumpPrepend() << "failed to uninstall package: " << pkg
                  << "\n";
        return false;
      }
    } else if (step_type == SOLVER_TRANSACTION_IGNORE) {
      // std::cout << "Ignoring: " << pool_id2str(pool, s->name) << "="
      //           << pool_id2str(pool, s->evr) << "\n";
    } else if (step_type == SOLVER_TRANSACTION_UPGRADED ||
               step_type == SOLVER_TRANSACTION_DOWNGRADED) {
      // 's' is the OLD package being removed.
      // need to ask libsolv what the NEW package is
      Id new_p = transaction_obs_pkg(trans, p);
      Solvable *new_s = pool_id2solvable(pool, new_p);

      std::string action = (step_type == SOLVER_TRANSACTION_UPGRADED)
                               ? "Upgrade: "
                               : "Downgrade: ";

      std::cout << chumpPrepend() << "" << action << pool_id2str(pool, s->name)
                << " from " << pool_id2str(pool, s->evr) << " to "
                << pool_id2str(pool, new_s->evr) << "\n";

      // install the new package
      string pkg = string(pool_id2str(pool, new_s->name));
      string pkgstr = pkg + "=" + string(pool_id2str(pool, new_s->evr));

      if (!uninstall(pkg, false)) {
        std::cerr << chumpPrepend() << "failed to uninstall package: " << pkg
                  << "\n";
        return false;
      }
      if (!install(pkgstr)) {
        std::cerr << chumpPrepend() << "failed to install package: " << pkg
                  << "\n";
      }
    } else {
      std::cout << chumpPrepend() << "unaccounted for step type: " << step_type
                << "\n";
    }
  }

  // Clean up
  transaction_free(trans);
  solver_free(solver);
  queue_free(&job);

  return true;
}

bool Manager::install(string packageName) {
  // parse packages here
  auto [name, version_string] = parsePackageName(packageName);

  // lookup package name (default to latest version)
  auto pkg = package_list->find_package(name);

  if (!pkg) {
    std::cerr << chumpPrepend() << "package " << packageName << " not found."
              << std::endl;
    return false;
  }

  Package package = pkg.value();

  optional<PackageVersion> ver;
  if (version_string) {
    try {
      PackageVersion pkgver(version_string.value());
      ver = package.version(pkgver, os, language_version, api_version);
    } catch (std::invalid_argument &e) {
      std::cerr << chumpPrepend() << "" << e.what() << '\n';
      return false;
    }
  } else {
    ver = package.latest_version(os, arch, language_version, api_version);
  }

  if (!ver) {
    std::cerr << chumpPrepend() << "unable to find version of package "
              << package.name << " that works on your system" << std::endl;

    return false;
  }

  PackageVersion version = ver.value();

  // if there is already a packages/PACKAGE directory, error out and tell the
  // user to call update
  fs::path install_dir = packagePath(package, chump_dir);

  if (fs::exists(install_dir / "version.json")) {
    std::cerr << chumpPrepend() << "the package '" << package.name
              << "' already exists." << std::endl;
    std::cerr << chumpPrepend() << "use `chump update " << package.name
              << "' to update the existing package" << std::endl;
    std::cerr << chumpPrepend() << "or use `chump uninstall " << package.name
              << "` to remove the package" << std::endl;
    return false;
  }

  // Create a temporary directory to download our files to
  fs::path temp_dir = makeTempDir();

  // fetch
  for (auto file : version.files) {
    fs::path dir = file.local_dir;
    string url = file.url;
    string checksum = file.checksum;
    FileType file_type = file.file_type;

    bool result =
        fetch->fetch(url, dir, package, version, temp_dir, file_type, checksum);
    if (!result) {
      std::cerr << chumpPrepend() << "failed to fetch " << url << ", exiting."
                << std::endl;
      return false;
    }
  }

  // If any of our fetched files are zip files, unzip them and then
  // delete the original zip file so it won't be copied over to the
  // packages dir.
  for (auto file : version.files) {
    if (file.file_type != ZIP_FILE)
      continue;

    fs::path filename = fs::path(file.url).filename();
    fs::path dir = temp_dir / file.local_dir;

    // Extract the file
    if (!unzipFile((dir / filename).string(), dir.string()))
      return false;

    fs::remove(dir / filename);
  }

  // Add all files to the InstalledVersion file list
  InstalledVersion installed_ver = InstalledVersion(package, version);
  for (auto const &dir_entry : fs::recursive_directory_iterator(temp_dir)) {
    if (fs::is_regular_file(dir_entry)) {
      installed_ver.files.push_back(fs::relative(dir_entry, temp_dir));
    }
  }

  // create install dir if needed
  fs::create_directory(install_dir);

  // Copy temp files over to the install directory
  try {
    fs::copy(temp_dir, install_dir,
             fs::copy_options::recursive |
                 fs::copy_options::overwrite_existing);
  } catch (std::filesystem::filesystem_error &e) {
    std::cerr << chumpPrepend() << "" << e.what() << '\n';
    return false;
  }

  // Removing temp dir
  fs::remove_all(temp_dir);

  // Write version.json to file.
  json version_json = installed_ver;

  std::ofstream o(install_dir / "version.json");
  o << std::setw(4) << version_json << std::endl;
  o.close();

  // We succeeded
  return true;
}

// Install a local package (with dependencies)
bool Manager::install_local_with_deps(fs::path pkgDefn, fs::path pkgVer,
                                      fs::path pkgZip) {
  /*
    Because the local package to be installed is neither the system
    nor the manifest repo (that's the entire point!), you cannot ran a
    solver on it. Instead, we will run a solver that's constrained on
    all of its dependencies. Those dependencies will be resolved and
    installed similarly to installWithDeps. And if the installation of
    all of the dependencies succeeds, then install_local(...) will be
    called with our package. ezpz
  */

  if (!fs::is_regular_file(pkgDefn)) {
    std::cerr << chumpPrepend() << "package definition " << pkgDefn
              << "is not a file, exiting..." << std::endl;
    return false;
  }

  if (!fs::is_regular_file(pkgVer)) {
    std::cerr << chumpPrepend() << "package version " << pkgVer
              << "is not a file, exiting..." << std::endl;
    return false;
  }

  if (!fs::is_regular_file(pkgZip)) {
    std::cerr << chumpPrepend() << "package zip " << pkgZip
              << "is not a file, exiting..." << std::endl;
    return false;
  }

  // only supporting zip for now
  if (pkgZip.extension() != ".zip") {
    std::cerr << chumpPrepend() << "package path " << pkgZip
              << "is not a zip file, exiting..." << std::endl;
    return false;
  }

  // attempt to open the package files
  auto version = open_package_version_file(pkgVer);
  if (!version)
    return false;

  optional<Package> package = open_package_file(pkgDefn);
  if (!package)
    return false;

  // Do dependency resolution
  Queue job;
  queue_init(&job);

  // add dep constraints to solver
  for (auto dep : version.value().dependencies) {
    Id name_id = pool_str2id(pool, dep.name.c_str(), 1);

    // optionally, add min/max version bounds of the dependency
    if (dep.version_min) {
      Id evr_id = pool_str2id(
			      pool, dep.version_min.value().getVersionString().c_str(), 1);

      // create an relational ID (i.e. "Rec >= 1.1.0")
      Id rel_id = pool_rel2id(pool, name_id, evr_id, REL_GT | REL_EQ, 1);
      queue_push2(&job, SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES, rel_id);
    }

    if (dep.version_max) {
      Id evr_id = pool_str2id(
			      pool, dep.version_max.value().getVersionString().c_str(), 1);

      // create an relational ID (i.e. "Rec <= 1.1.0")
      Id rel_id = pool_rel2id(pool, name_id, evr_id, REL_LT | REL_EQ, 1);
      queue_push2(&job, SOLVER_INSTALL | SOLVER_SOLVABLE_PROVIDES, rel_id);
    }

    if (!dep.version_min && !dep.version_max) {
      queue_push2(&job, SOLVER_INSTALL | SOLVER_SOLVABLE_NAME, name_id);
    }
  }

  // Create and run solver
  Solver *solver = solver_create(pool);

  // Allow the solver to downgrade installed packages if a dependency requires
  // it
  solver_set_flag(solver, SOLVER_FLAG_ALLOW_DOWNGRADE, 1);

  // Solve it!
  solver_solve(solver, &job);

  // check for solver success
  int problem_count = solver_problem_count(solver);
  if (problem_count > 0) {
    std::cerr << chumpPrepend() << "dependency resolution failed with "
              << problem_count << " problems:\n";

    Id problem = 0;
    // Pass 0 to get the first problem, then pass the previous ID to get the
    // next
    while ((problem = solver_next_problem(solver, problem)) != 0) {
      std::cerr << " - " << solver_problem2str(solver, problem) << "\n";
    }

    return false;
  }

  // parse the transaction and install/erase/upgrade
  Transaction *trans = solver_create_transaction(solver);

  // topologically order transactions
  transaction_order(trans, 0);

  for (int i = 0; i < trans->steps.count; ++i) {
    Id p = trans->steps.elements[i];
    Solvable *s = pool_id2solvable(pool, p);

    // Determine what action the solver is actually taking on this package
    // Id step_type = transaction_type(trans, p, SOLVER_TRANSACTION_SHOW_ALL);
    Id step_type = transaction_type(trans, p, 0);

    // ignore ck & api version (we can't install these!)
    if (strcmp(pool_id2str(pool, s->name), "chuck-system-version") == 0)
      continue;
    if (strcmp(pool_id2str(pool, s->name), "chuck-system-api") == 0)
      continue;

    if (step_type == SOLVER_TRANSACTION_INSTALL) {
      string s_name = pool_id2str(pool, s->name);
      string s_evr = pool_id2str(pool, s->evr);

      // Print different message depending on if it's dependency or not
      std::cout << chumpPrepend()
		<< "installing dependency: " << packageStr(s_name, s_evr)
		<< "\n";

      string pkg = string(pool_id2str(pool, s->name));
      string ver = string(pool_id2str(pool, s->evr));
      string pkgstr = pkg + "=" + ver;

      // do installation here
      if (!install(pkgstr)) {
	std::cerr << chumpPrepend() << "failed to install dependency " << packageStr(pkg, ver) << "\n";
      }
    } else if (step_type == SOLVER_TRANSACTION_ERASE) {
      std::cout << chumpPrepend()
                << "removing package: " << pool_id2str(pool, s->name) << "="
                << pool_id2str(pool, s->evr) << "\n";

      string pkg = string(pool_id2str(pool, s->name));
      string pkgstr = pkg + "=" + string(pool_id2str(pool, s->evr));

      if (!uninstall(pkg, false)) {
        std::cerr << chumpPrepend() << "failed to uninstall package: " << pkg
                  << "\n";
        return false;
      }
    } else if (step_type == SOLVER_TRANSACTION_IGNORE) {
      // std::cout << "Ignoring: " << pool_id2str(pool, s->name) << "="
      //           << pool_id2str(pool, s->evr) << "\n";
    } else if (step_type == SOLVER_TRANSACTION_UPGRADED ||
               step_type == SOLVER_TRANSACTION_DOWNGRADED) {
      // 's' is the OLD package being removed.
      // need to ask libsolv what the NEW package is
      Id new_p = transaction_obs_pkg(trans, p);
      Solvable *new_s = pool_id2solvable(pool, new_p);

      std::string action = (step_type == SOLVER_TRANSACTION_UPGRADED)
                               ? "Upgrade: "
                               : "Downgrade: ";

      std::cout << chumpPrepend() << "" << action << pool_id2str(pool, s->name)
                << " from " << pool_id2str(pool, s->evr) << " to "
                << pool_id2str(pool, new_s->evr) << "\n";

      // install the new package
      string pkg = string(pool_id2str(pool, s->name));
      string ver = string(pool_id2str(pool, s->evr));
      string pkgstr = pkg + "=" + ver;

      // uninstall
      if (!uninstall(pkg, false)) {
        std::cerr << chumpPrepend() << "failed to uninstall package: " << pkg
                  << "\n";
        return false;
      }

      // instsall
      if (!install(pkgstr)) {
	std::cerr << chumpPrepend() << "failed to install dependency " << packageStr(pkg, ver) << "\n";
      }
    } else {
      std::cout << chumpPrepend() << "unaccounted for step type: " << step_type
                << "\n";
    }
  }

  // Clean up
  // TODO: if we return early these will not be cleaned up. Need to make an
  // error goto to handle this. However, this isn't super high priority as
  // chump is only going to run this (or installWithDeps) once before exit.
  transaction_free(trans);
  solver_free(solver);
  queue_free(&job);

  // if all of the dependencies sucessfully
  return install_local(pkgDefn, pkgVer, pkgZip);
}


// installing locally with the package dir (to be called from install_local_with_deps)
bool Manager::install_local(fs::path pkgDefn, fs::path pkgVer,
                            fs::path pkgZip) {
  if (!fs::is_regular_file(pkgDefn)) {
    std::cerr << chumpPrepend() << "package definition " << pkgDefn
              << "is not a file, exiting..." << std::endl;
    return false;
  }

  if (!fs::is_regular_file(pkgVer)) {
    std::cerr << chumpPrepend() << "package version " << pkgVer
              << "is not a file, exiting..." << std::endl;
    return false;
  }

  if (!fs::is_regular_file(pkgZip)) {
    std::cerr << chumpPrepend() << "package zip " << pkgZip
              << "is not a file, exiting..." << std::endl;
    return false;
  }

  // only supporting zip for now
  if (pkgZip.extension() != ".zip") {
    std::cerr << chumpPrepend() << "package path " << pkgZip
              << "is not a zip file, exiting..." << std::endl;
    return false;
  }

  // attempt to open the package files
  auto version = open_package_version_file(pkgVer);
  if (!version)
    return false;

  optional<Package> package = open_package_file(pkgDefn);
  if (!package)
    return false;

  // if there is already a packages/PACKAGE directory, error out and tell the
  // user to call update
  fs::path install_dir = packagePath(package.value(), chump_dir);

  if (fs::exists(install_dir / "version.json")) {
    std::cerr << chumpPrepend() << "the package '" << package.value().name
              << "' already exists." << std::endl;
    std::cerr << chumpPrepend() << "uninstalling '" << package.value().name
              << "'..." << std::endl;
    uninstall(package.value().name, false);
  }

  std::cerr << chumpPrepend() << "installing '" << package.value().name
            << "'..." << std::endl;
  fs::create_directory(install_dir);
  // Unzip the local file to the installed directory
  if (!unzipFile(pkgZip.string(), install_dir.string()))
    return false;

  // Add all files to the InstalledVersion file list
  InstalledVersion installed_ver =
      InstalledVersion(package.value(), version.value());
  for (auto const &dir_entry : fs::recursive_directory_iterator(install_dir)) {
    if (fs::is_regular_file(dir_entry)) {
      installed_ver.files.push_back(fs::relative(dir_entry, install_dir));
    }
  }

  // Write version.json to file.
  json version_json = installed_ver;

  std::ofstream o(install_dir / "version.json");
  o << std::setw(4) << version_json << std::endl;
  o.close();

  // We succeeded
  return true;
}

bool Manager::update(string packageName) {
  // lookup package name (default to latest version)
  auto pkg = package_list->find_package(packageName);

  if (!pkg) {
    std::cerr << chumpPrepend() << "package " << packageName << " not found."
              << std::endl;
    return false;
  }

  Package package = pkg.value();

  // if there is already a .chump/PACKAGE directory, error out and tell the user
  // to call update
  fs::path install_dir = packagePath(package, chump_dir);

  if (!fs::exists(install_dir)) {
    std::cerr << chumpPrepend() << "the install directory '" << install_dir
              << "' does not exist." << std::endl;
    std::cerr << chumpPrepend() << "use `chump install " << package.name
              << "' to install the existing package" << std::endl;
    return false;
  }

  optional<InstalledVersion> installed_version =
      open_installed_version_file(install_dir / "version.json");

  if (!installed_version) {
    std::cerr << chumpPrepend() << "the version definition '"
              << install_dir / "version.json" << "' does not exist."
              << std::endl;
    std::cerr << chumpPrepend() << "try removing the directory " << install_dir
              << std::endl;
    std::cerr << chumpPrepend() << "and then calling 'chump install '"
              << packageName << "'\n";
    return false;
  }

  PackageVersion curr_version = installed_version.value().version();

  optional<PackageVersion> ver =
      package.latest_version(os, arch, language_version, api_version);

  if (!ver) {
    std::cerr << chumpPrepend() << "unable to find version of package "
              << package.name << " that works on your system" << std::endl;

    return false;
  }

  PackageVersion latest_version = ver.value();

  if (curr_version == latest_version) {
    std::cerr << chumpPrepend() << "" << package.name
              << " is already up-to-date." << std::endl;
    return true;
  }

  if (curr_version > latest_version) {
    std::cerr << chumpPrepend() << "" << package.name
              << " is installed already with a newer version." << std::endl;
    return true;
  }

  // Create a temporary directory to download our files to
  fs::path temp_dir = makeTempDir();

  // fetch
  for (auto file : latest_version.files) {
    fs::path dir = file.local_dir;
    string url = file.url;
    string checksum = file.checksum;
    FileType file_type = file.file_type;

    bool result = fetch->fetch(url, dir, package, latest_version, temp_dir,
                               file_type, checksum);
    if (!result) {
      std::cerr << chumpPrepend() << "failed to fetch " << url << ", exiting."
                << std::endl;
      return false;
    }
  }

  // If any of our fetched files are zip files, unzip them and then
  // delete the original zip file so it won't be copied over to the
  // packages dir.
  for (auto file : latest_version.files) {
    if (file.file_type != ZIP_FILE)
      continue;

    fs::path filename = fs::path(file.url).filename();
    fs::path dir = temp_dir / file.local_dir;

    // Extract the file
    if (!unzipFile((dir / filename).string(), dir.string()))
      return false;

    fs::remove(dir / filename);
  }

  InstalledVersion installed_ver(package, latest_version);
  for (auto const &dir_entry : fs::recursive_directory_iterator(temp_dir)) {
    if (fs::is_regular_file(dir_entry)) {
      installed_ver.files.push_back(fs::relative(dir_entry, temp_dir));
    }
  }

  // uninstall our current version
  // this is done as late as possible so that if fetching/extracting the updated
  // package fails, we won't have removed the current installation prematurely.
  for (auto file : installed_version.value().files) {
    fs::path curr_dir = (install_dir / file.parent_path()).lexically_normal();
    fs::path curr_file = (install_dir / file).lexically_normal();

    if (fs::exists(curr_file)) {
      fs::remove(curr_file);
    } else {
      std::cerr << chumpPrepend() << "attempted to remove " << curr_file
                << ", but the file was not found, continuing..." << std::endl;
    }

    if (fs::is_empty(curr_dir)) {
      fs::remove(curr_dir);
    }
  }

  // create install dir if needed
  fs::create_directory(install_dir);

  // Copy temp files over to the install directory
  try {
    fs::copy(temp_dir, install_dir,
             fs::copy_options::recursive |
                 fs::copy_options::overwrite_existing);
  } catch (fs::filesystem_error &e) {
    std::cerr << chumpPrepend() << "" << e.what() << '\n';
    return false;
  }

  // Write version.json to file.
  json latest_version_json = installed_ver;
  std::ofstream o(install_dir / "version.json");
  o << std::setw(4) << latest_version_json << std::endl;
  o.close();

  return true;
}

//-----------------------------------------------------------------------------
// name: uninstall_with_deps()
// desc: Uninstall the package with dependency managment. If another
//       package depends on the package you're trying to uninstall, it
//       will tell you to uninstall those first. At the moment this can't
//       uninstall dependencies as we don't currently track what was
//       installed as a dependency and what wasn't.
//-----------------------------------------------------------------------------
bool Manager::uninstall_with_deps(string packageName, bool force) {
  Queue job;
  queue_init(&job);

  auto [name, version_string] = parsePackageName(packageName);

  // define job
  Id name_id = pool_str2id(pool, name.c_str(), 1);

  queue_push2(&job, SOLVER_ERASE | SOLVER_SOLVABLE_NAME, name_id);

  // std::cout << "--- libsolv Pool State Dump ---\n";
  // // Solvables are stored in a contiguous array from index 1 to
  // // pool->nsolvables
  // for (int i = 1; i < pool->nsolvables; i++) {
  //   Solvable *s = pool->solvables + i;

  //   // Only print actual packages, skip empty slots
  //   if (s->repo) {
  //     std::cout << "Repo: " << s->repo->name
  // 		<< " | Pkg: '" << pool_id2str(pool, s->name) << "'"
  // 		<< " ver '" << pool_id2str(pool, s->evr) << "'"
  // 		<< " (Name ID: " << s->name << ")\n";
  //   }
  // }
  // std::cout << "-------------------------------\n";

  // Mark all system packages as locked, so that the solver cannot
  // continue if the uninstalling package is a dependency of anything
  for (int i = 1; i < pool->nsolvables; i++) {
    Solvable *s = pool->solvables + i;

    if (s->repo && strcmp(s->repo->name, "@System") == 0 &&
        s->name != name_id) {
      queue_push2(&job, SOLVER_LOCK | SOLVER_SOLVABLE_NAME, s->name);
    }
  }

  // Empty all non-system repos so the solver cannot use them as fallbacks
  for (int i = 0; i < pool->nrepos; i++) {
    Repo *repo = pool->repos[i];
    if (repo && strcmp(repo->name, "@System") != 0) {
      repo_empty(repo, 0);
    }
  }

  // Rebuild the lookup index without the manifest packages
  pool_createwhatprovides(pool);

  // Create and run solver
  Solver *solver = solver_create(pool);

  // might need to set SOLVER_FLAG_ALLOW_UNINSTALL
  solver_set_flag(solver, SOLVER_FLAG_ALLOW_UNINSTALL, 1);

  // Solve it!
  solver_solve(solver, &job);

  // check for solver success
  int problem_count = solver_problem_count(solver);
  if (problem_count > 0) {
    std::cerr << chumpPrepend();
    if (force) {
      std::cerr << TC::red("WARNING") << ": ";
    }

    std::cerr << "dependency resolution failed while attempting to uninstall " << packageName << " with ";

    if (problem_count > 1)
      std::cerr << problem_count << " problems:\n";
    else std::cerr << "1 problem:\n";

    Id problem = 0;
    // Pass 0 to get the first problem, then pass the previous ID to get the
    // next
    while ((problem = solver_next_problem(solver, problem)) != 0) {
      std::cerr << " - " << solver_problem2str(solver, problem) << "\n";
    }

    if (force) {
      std::cerr << chumpPrepend() << "attempting to uninstall anyways...\n";
      return uninstall(packageName, force);
    }

    return false;
  }

  // parse the transaction and install/erase/upgrade
  // there are a lot of options:
  // https://github.com/openSUSE/libsolv/blob/c9ee7a3b946e8870f056dddc22032e4d43a40636/src/transaction.h#L45
  Transaction *trans = solver_create_transaction(solver);

  // No steps means that there's nothing to be done, which means that the package wasn't installed
  if (trans->steps.count == 0) {
    if (!force) {
      std::cerr << chumpPrepend() << "the package " << TC::bold(packageName)
                << " was not found, could not uninstall" << std::endl;
    }

    transaction_free(trans);
    solver_free(solver);
    queue_free(&job);

    // if we are forcing, we're gonna try anyway (because the version.json could have been deleted)
    if (force) {
      return uninstall(packageName, force);
    }
  }

  // topologically order transactions
  // transaction_order(trans, 0);

  // Because we are not uninstalling dependencies we shouldn't need to traverse
  // the solver's steps. It's just for validating.
  transaction_free(trans);
  solver_free(solver);
  queue_free(&job);

  return uninstall(packageName, force);
}

bool Manager::uninstall(string packageName, bool force) {
  // if there isn't already a .chump/PACKAGE directory, error out and tell the
  // user to install
  fs::path packageNamePath = fs::path(packageName);
  fs::path install_dir = chump_dir / packageNamePath;

  if (!fs::exists(install_dir)) {
    std::cerr << chumpPrepend() << "the install directory '" << install_dir
              << "' does not exist." << std::endl;
    std::cerr << chumpPrepend() << "use `chump install " << packageName
              << "' to install the existing package" << std::endl;
    return false;
  }

  std::cout << chumpPrepend()
	    << TC::blue("uninstalling package", TRUE) << ": " << packageStr(packageName)
	    << "\n";

  if (force) {
    std::cerr << chumpPrepend() << "force-uninstall " << packageName
              << " (completely removing the installation directory)...\n";
    fs::remove_all(install_dir.lexically_normal());
    return true;
  }

  optional<InstalledVersion> installed_ver =
      open_installed_version_file(install_dir / "version.json");

  if (!installed_ver)
    return false;

  PackageVersion curr_ver = installed_ver.value().version();

  // Remove all files associated with package
  for (auto file : installed_ver.value().files) {
    fs::path filepath = file.lexically_normal();

    fs::path full_path = install_dir / filepath;

    if (fs::exists(full_path)) {
         fs::remove(full_path);

      std::string line = string("   └─[") + TC::green("REMOVED", TRUE) + "] " +
                     TC::blue(full_path);
      std::cerr << chumpPrepend() << line << std::endl;
    } else {
      std::cerr << chumpPrepend() << string("   └─[") << TC::red("ERROR", TRUE)
                << "] "
                << "attempted to remove " << TC::blue(full_path) << "," << std::endl;
      std::cerr << "                     "
                << "but the file was not found, continuing..." << std::endl;

    }

    fs::path parent = filepath.parent_path();
    fs::path root = filepath.root_path();

    fs::path parent_path = install_dir / parent;

    // make sure the parent dir isn't the root dir and check that it's
    // empty
    if (parent != root && fs::is_empty(parent_path)) {
      fs::remove(parent_path.lexically_normal());
    }
  }

  fs::remove(install_dir / "version.json"); // This is generated by chump

  // Clean up the directory if it's empty. If there's stuff there that
  // was added not by the installer, treat that as not-touched so that
  // if you reinstall the package your files will still be there.
  if (fs::is_empty(install_dir)) {
    fs::remove(install_dir.lexically_normal());
  }

  return true;
}

//-----------------------------------------------------------------------------
// name: listPackages()
// desc: Return a list of all packages. This includes both packages found
//       on the manifest as well as packages that were installed from a
//       local source
//-----------------------------------------------------------------------------
std::vector<Package> Manager::listPackages() {
  std::vector<Package> pkg_list = package_list->get_packages();

  for (auto const &dir_entry : fs::directory_iterator{chump_dir}) {
    // To be installed, a package has to have a direction in the chump_dir
    // and that directory has to contain a version.json
    if (!fs::is_directory(dir_entry))
      continue;
    if (!fs::exists(dir_entry.path() / "version.json"))
      continue;

    optional<InstalledVersion> installed_version =
        getInstalledVersion(dir_entry);

    // If not installed, skip
    if (!installed_version)
      continue;

    bool found_pkg = false;
    for (Package pkg : pkg_list) {
      if (pkg.name == installed_version.value().name) {
        found_pkg = true;
        break;
      }
    }

    if (!found_pkg) {
      pkg_list.push_back(installed_version.value().package());
    }

    // std::cout << dir_entry << std::endl;
  }

  return pkg_list;
}

//-----------------------------------------------------------------------------
// name: listInstalledVersions()
// desc: Return a list of all currently installed package versions. These do not
//       have to be on the manifest (i.e. can be installed from a local source)
//-----------------------------------------------------------------------------
std::vector<InstalledVersion> Manager::listInstalledVersions() {
  std::vector<InstalledVersion> version_list;

  for (auto const &dir_entry : fs::directory_iterator{chump_dir}) {
    // To be installed, a package has to have a direction in the chump_dir
    // and that directory has to contain a version.json
    if (!fs::is_directory(dir_entry))
      continue;
    if (!fs::exists(dir_entry.path() / "version.json"))
      continue;

    optional<InstalledVersion> installed_version =
        getInstalledVersion(dir_entry);

    // If not installed, skip
    if (!installed_version)
      continue;

    version_list.push_back(installed_version.value());
  }

  return version_list;
}

bool Manager::is_installed(Package pkg) {
  fs::path install_dir = packagePath(pkg, chump_dir);

  return fs::exists(install_dir / "version.json");
}

// Returns the path where a package would be installed (if it exists)
fs::path Manager::install_path(Package pkg) {
  return packagePath(pkg, chump_dir);
}

string Manager::getSystemChuckVersion() {
  return language_version.getVersionString();
};

bool Manager::update_manifest() {
  optional<int> newest_manifest_version = fetch->fetch_newest_manifest_version(
      "https://chuck.stanford.edu/release/chump/manifest/"
      "latest-manifest.version");

  if (newest_manifest_version &&
      newest_manifest_version.value() > MANIFEST_VERSION_NO) {
    std::cerr
        << chumpPrepend()
        << "there are newer packages available that this version of "
           "chump cannot manage. visit https://chuck.stanford.edu/release/ and "
           "install the latest version of chuck (which includes an updated "
           "version of chump). once you have done this, rerun chump for the "
           "latest, greatest, chumpiest experience available\n";
    std::cerr << chumpPrepend() << "~~ don't be a lump, update chump ~~\n";
  }

  // Create a temporary directory to download our manifest to
  fs::path temp_dir = makeTempDir();

  // load both tmp file and current file as strings
  bool result = fetch->fetch_manifest(manifest_url, temp_dir);

  if (!result) {
    std::cerr << chumpPrepend()
              << "failed to fetch manifest.json, continuing..." << std::endl;
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

  while (temp_manifest_iter != end && curr_manifest_iter != end) {
    if (*temp_manifest_iter != *curr_manifest_iter) {
      are_equal = false;
      break;
    }
    ++temp_manifest_iter;
    ++curr_manifest_iter;
  }

  if ((temp_manifest_iter != end) || (curr_manifest_iter != end)) {
    are_equal = false;
  }

  temp_manifest.close();
  curr_manifest.close();

  if (are_equal) {
    // since this is now being updated every time chump is called (except for
    // help), we are now being silent. std::cerr << chumpPrepend() << "current
    // manifest is up-to-date, doing nothing"
    //           << std::endl;
    return false;
  }

  // Copy temp files over to the install directory
  try {
    fs::remove(chump_dir / "manifest.json");
    fs::copy(temp_dir / "manifest.json", chump_dir / "manifest.json",
             fs::copy_options::overwrite_existing);
  } catch (std::filesystem::filesystem_error &e) {
    std::cerr << chumpPrepend() << e.what() << '\n';
    return false;
  }

  std::cerr << chumpPrepend() << "manifest.json was successfully updated!"
            << std::endl;
  return true;
}

bool Manager::open_doc(string packageName) {
  fs::path packageNamePath = fs::path(packageName);
  fs::path installDir = chump_dir / packageNamePath;

  if (!fs::exists(installDir)) {
    // get optional package
    optional<Package> pkg = getPackage(packageName);
    // check
    if (!pkg) {
      std::cerr << chumpPrepend() << "the package '" << packageName
                << "' does not exist." << std::endl;
      return false;
    } else {
      std::cerr << chumpPrepend() << "the install directory '" << installDir
                << "' does not exist." << std::endl;
      std::cerr << chumpPrepend() << "use `chump install " << packageName
                << "' to install the existing package" << std::endl;
      return false;
    }
  }

  auto indexPath = installDir / fileTypeToDir(DOCS_FILE) / "index.html";

  // the package is installed. check if there is an index.html
  if (!fs::exists(indexPath)) {
    std::cerr << chumpPrepend() << "package '" << packageName
              << "' does not have documentation. sorry!" << std::endl;
    return false;
  }

  openDocCmd(indexPath);
  // std::ostringstream cmd;
  // cmd << "open " << indexPatggh;
  // system(cmd.str().c_str());

  return true;
}

bool Manager::open_examples(string packageName) {
  fs::path packageNamePath = fs::path(packageName);
  fs::path installDir = chump_dir / packageNamePath;

  if (!fs::exists(installDir)) {
    // get optional package
    optional<Package> pkg = getPackage(packageName);
    // check
    if (!pkg) {
      std::cerr << chumpPrepend() << "the package '" << packageName
                << "' does not exist." << std::endl;
      return false;
    } else {
      std::cerr << chumpPrepend() << "the install directory '" << installDir
                << "' does not exist." << std::endl;
      std::cerr << chumpPrepend() << "use `chump install " << packageName
                << "' to install the existing package" << std::endl;
      return false;
    }
  }
  auto examplePath = installDir / fileTypeToDir(EXAMPLE_FILE);

  // the package is installed. check if there is an index.html
  if (!fs::exists(examplePath)) {
    std::cerr << chumpPrepend() << "package '" << packageName
              << "' does not have any examples. sorry!" << std::endl;
    return false;
  }

  openExamplesCmd(examplePath);
  return true;
}

optional<Package> Manager::open_package_file(fs::path path) {
  // parse package and version files
  std::ifstream ver_stream(path);
  if (!ver_stream.good()) {
    std::cerr << chumpPrepend() << "unable to open \"" << path << "\""
              << std::endl;
    ver_stream.close();
    return {};
  }

  json pkg_ver;
  // need to close file on failure
  try {
    pkg_ver = json::parse(ver_stream);
    ver_stream.close();
  } catch (const std::exception &e) {
    ver_stream.close();
    std::cerr << chumpPrepend() << "unable to parse \"" << path
              << "\": " << e.what() << std::endl;
    return {};
  }

  try {
    Package version = pkg_ver.template get<Package>();
    ver_stream.close();
    return version;
  } catch (const std::exception &e) {
    ver_stream.close();
    std::cerr << chumpPrepend() << "unable to cast \"" << path
              << "\" to Package: " << e.what() << std::endl;
    return {};
  }
}

optional<InstalledVersion> Manager::open_installed_version_file(fs::path path) {
  // parse package and version files
  std::ifstream ver_stream(path);
  if (!ver_stream.good()) {
    // std::cerr << chumpPrepend() << "unable to open \"" << path << "\"" <<
    // std::endl;
    ver_stream.close();
    return {};
  }

  json pkg_ver;
  // need to close file on failure
  try {
    pkg_ver = json::parse(ver_stream);
    ver_stream.close();
  } catch (const std::exception &e) {
    ver_stream.close();
    std::cerr << chumpPrepend() << "unable to parse \"" << path
              << "\": " << e.what() << std::endl;
    return {};
  }

  try {
    InstalledVersion version = pkg_ver.template get<InstalledVersion>();
    ver_stream.close();
    return version;
  } catch (const std::exception &e) {
    ver_stream.close();
    std::cerr << chumpPrepend() << "unable to cast \"" << path
              << "\" to InstalledVersion: " << e.what() << std::endl;
    return {};
  }
}

optional<PackageVersion> Manager::open_package_version_file(fs::path path) {
  // parse package and version files
  std::ifstream ver_stream(path);
  if (!ver_stream.good()) {
    std::cerr << chumpPrepend() << "unable to open \"" << path << "\""
              << std::endl;
    ver_stream.close();
    return {};
  }

  json pkg_ver;
  // need to close file on failure
  try {
    pkg_ver = json::parse(ver_stream);
    ver_stream.close();
  } catch (const std::exception &e) {
    ver_stream.close();
    std::cerr << chumpPrepend() << "unable to parse \"" << path
              << "\": " << e.what() << std::endl;
    return {};
  }

  try {
    PackageVersion version = pkg_ver.template get<PackageVersion>();
    ver_stream.close();
    return version;
  } catch (const std::exception &e) {
    ver_stream.close();
    std::cerr << chumpPrepend() << "unable to cast \"" << path
              << "\" to PackageVersion: " << e.what() << std::endl;
    return {};
  }
}

//-----------------------------------------------------------------------------
// name: initializeSystemRepo()
// desc: Create a libsolv repo of all the packages installed on your system
//       (and their dependencies)
//-----------------------------------------------------------------------------
Repo *Manager::initializeSystemRepo() {
  Repo *repo = repo_create(pool, "@System");
  pool_set_installed(pool,
                     repo); // mark @System as the repo of installed packages

  std::vector<InstalledVersion> installed_versions = listInstalledVersions();

  // std::cout << "initiazle system repo\n";

  // Add chuck's language information as a constraint in the repo.
  Id ck_lang_id = repo_add_solvable(repo);
  Solvable *s_lang = pool_id2solvable(pool, ck_lang_id);

  s_lang->name = pool_str2id(pool, "chuck-system-version", 1);
  s_lang->evr =
      pool_str2id(pool, language_version.getVersionString().c_str(), 1);

  // Provide the virtual capability (e.g. "chuck-version = 1.5.5.9"
  Id cap_ck_name = pool_str2id(pool, "chuck-version", 1);
  Id cap_ck_ver =
      pool_str2id(pool, language_version.getVersionString().c_str(), 1);
  Id prov_rel_ck = pool_rel2id(pool, cap_ck_name, cap_ck_ver, REL_EQ, 1);

  solvable_add_deparray(s_lang, SOLVABLE_PROVIDES, prov_rel_ck, 0);

  // Add chuck api information as a constraint
  Id ck_api_id = repo_add_solvable(repo);
  Solvable *s_api = pool_id2solvable(pool, ck_api_id);

  s_api->name = pool_str2id(pool, "chuck-system-api", 1);
  s_api->evr = pool_str2id(pool, api_version.getVersionString().c_str(), 1);

  // Provide the virtual capability (e.g. "chuck-version = 1.5.5.9"
  Id cap_api_name = pool_str2id(pool, "chuck-api", 1);
  Id cap_api_ver = pool_str2id(pool, api_version.getVersionString().c_str(), 1);
  Id prov_rel_api = pool_rel2id(pool, cap_api_name, cap_api_ver, REL_EQ, 1);

  solvable_add_deparray(s_api, SOLVABLE_PROVIDES, prov_rel_api, 0);

  // add all installed packages to system repo
  for (auto ver : installed_versions) {
    Id pkgId = repo_add_solvable(repo);
    Solvable *solv = pool_id2solvable(pool, pkgId);

    solv->name = pool_str2id(pool, ver.name.c_str(), 1);
    solv->evr = pool_str2id(pool, ver.version().getVersionString().c_str(), 1);

    // this causes a pkg to conflict with itself, which means that
    // different versions of a package being installed at the same
    // time will conflict
    solvable_add_deparray(solv, SOLVABLE_CONFLICTS, solv->name, 0);

    string arch = architectureToString.at(ver.arch);
    solv->arch = pool_str2id(pool, arch.c_str(), 1);

    Id rel_provides = pool_rel2id(pool, solv->name, solv->evr, REL_EQ, 1);
    solvable_add_deparray(solv, SOLVABLE_PROVIDES, rel_provides, 0);

    // add chuck language version constraint
    Id min_ck_ver = pool_str2id(
        pool, ver.language_version_min.getVersionString().c_str(), 1);
    Id req_min_ck_ver =
        pool_rel2id(pool, cap_ck_name, min_ck_ver, REL_GT | REL_EQ, 1);
    solvable_add_deparray(solv, SOLVABLE_REQUIRES, req_min_ck_ver, 0);

    // set max lang version (if exists)
    if (ver.language_version_max) {
      Id max_ck_ver = pool_str2id(
          pool, ver.language_version_max.value().getVersionString().c_str(), 1);
      Id req_max_ck_ver =
          pool_rel2id(pool, cap_ck_name, max_ck_ver, REL_LT | REL_EQ, 1);
      solvable_add_deparray(solv, SOLVABLE_REQUIRES, req_max_ck_ver, 0);
    }

    // set api version (if exists)
    if (ver.api_version) {
      Id api_ver = pool_str2id(
          pool, ver.api_version.value().getVersionString().c_str(), 1);
      Id req_api_ver = pool_rel2id(pool, cap_api_name, api_ver, REL_EQ, 1);
      solvable_add_deparray(solv, SOLVABLE_REQUIRES, req_api_ver, 0);
    }

    // add dependencies (weeee)
    for (Dependency dep : ver.dependencies) {
      Id name_id = pool_str2id(pool, dep.name.c_str(), 1);

      // optionally, add min/max version bounds of the dependency
      if (dep.version_min) {
        Id evr_id = pool_str2id(
            pool, dep.version_min.value().getVersionString().c_str(), 1);

        // create an relational ID (i.e. "Rec >= 1.1.0")
        Id rel_id = pool_rel2id(pool, name_id, evr_id, REL_GT | REL_EQ, 1);
        solvable_add_deparray(solv, SOLVABLE_REQUIRES, rel_id, 0);
      }

      if (dep.version_max) {
        Id evr_id = pool_str2id(
            pool, dep.version_max.value().getVersionString().c_str(), 1);

        // create an relational ID (i.e. "Rec <= 1.1.0")
        Id rel_id = pool_rel2id(pool, name_id, evr_id, REL_LT | REL_EQ, 1);
        solvable_add_deparray(solv, SOLVABLE_REQUIRES, rel_id, 0);
      }

      if (!dep.version_min && !dep.version_max) {
        solvable_add_deparray(solv, SOLVABLE_REQUIRES, name_id, 0);
      }
    }
  }
  repo_internalize(repo);

  // Verify system repo setup
  // printf("Installed repo populated with %d solvables, %d ids.\n",
  //        repo->nsolvables, repo->idarraysize);

  return repo;
}

//-----------------------------------------------------------------------------
// name: initializeManifestRepo()
// desc: Create a libsolv repo of all packages available in the manifest
//-----------------------------------------------------------------------------
Repo *Manager::initializeManifestRepo() {
  Repo *repo = repo_create(pool, "manifest");

  // Enable debug printing
  // pool_setdebuglevel(pool, 2);  // 0 = nothing, 1 = minimal, 2 = verbose

  std::vector<Package> packages = listPackages();

  // std::cout << "initialize manifest repo\n";

  for (Package package : packages) {
    for (PackageVersion ver : package.versions) {
      Id pkgId = repo_add_solvable(repo);
      Solvable *solv = pool_id2solvable(pool, pkgId);

      solv->name = pool_str2id(pool, package.name.c_str(), 1);
      solv->evr = pool_str2id(pool, ver.getVersionString().c_str(), 1);

      // this causes a pkg to conflict with itself, which means that
      // different versions of a package being installed at the same
      // time will conflict
      solvable_add_deparray(solv, SOLVABLE_CONFLICTS, solv->name, 0);

      string arch = architectureToString.at(ver.arch);
      solv->arch = pool_str2id(pool, arch.c_str(), 1);

      Id rel_provides = pool_rel2id(pool, solv->name, solv->evr, REL_EQ, 1);
      solvable_add_deparray(solv, SOLVABLE_PROVIDES, rel_provides, 0);

      // add chuck language version constraint
      Id cap_ck_name = pool_str2id(pool, "chuck-version", 1);

      Id min_ck_ver = pool_str2id(
          pool, ver.language_version_min.getVersionString().c_str(), 1);
      Id req_min_ck_ver =
          pool_rel2id(pool, cap_ck_name, min_ck_ver, REL_GT | REL_EQ, 1);
      solvable_add_deparray(solv, SOLVABLE_REQUIRES, req_min_ck_ver, 0);

      // set max lang version (if exists)
      if (ver.language_version_max) {
        Id max_ck_ver = pool_str2id(
            pool, ver.language_version_max.value().getVersionString().c_str(),
            1);
        Id req_max_ck_ver =
            pool_rel2id(pool, cap_ck_name, max_ck_ver, REL_LT | REL_EQ, 1);
        solvable_add_deparray(solv, SOLVABLE_REQUIRES, req_max_ck_ver, 0);
      }

      // set api version (if exists)
      if (ver.api_version) {
        Id api_name = pool_str2id(pool, "chuck-api", 1);
        Id api_ver = pool_str2id(
            pool, ver.api_version.value().getVersionString().c_str(), 1);
        Id req_api_ver = pool_rel2id(pool, api_name, api_ver, REL_EQ, 1);
        solvable_add_deparray(solv, SOLVABLE_REQUIRES, req_api_ver, 0);
      }

      // add dependencies (weeee)
      for (Dependency dep : ver.dependencies) {
        // std::cout << "Dependency: " << dep.name << "\n";
        Id name_id = pool_str2id(pool, dep.name.c_str(), 1);

        // optionally, add min/max version bounds of the dependency
        if (dep.version_min) {
          Id evr_id = pool_str2id(
              pool, dep.version_min.value().getVersionString().c_str(), 1);

          // create an relational ID (i.e. "Rec >= 1.1.0")
          Id rel_id = pool_rel2id(pool, name_id, evr_id, REL_GT | REL_EQ, 1);
          solvable_add_deparray(solv, SOLVABLE_REQUIRES, rel_id, 0);
        }

        if (dep.version_max) {
          Id evr_id = pool_str2id(
              pool, dep.version_max.value().getVersionString().c_str(), 1);

          // create an relational ID (i.e. "Rec <= 1.1.0")
          Id rel_id = pool_rel2id(pool, name_id, evr_id, REL_LT | REL_EQ, 1);
          solvable_add_deparray(solv, SOLVABLE_REQUIRES, rel_id, 0);
        }

        if (!dep.version_min && !dep.version_max) {
          solvable_add_deparray(solv, SOLVABLE_REQUIRES, name_id, 0);
        }
      }
    }
  }
  repo_internalize(repo);

  // Verify system repo setup
  // printf("Installed repo populated with %d solvables, %d ids.\n",
  //        repo->nsolvables, repo->idarraysize);

  return repo;
}

void printDependencies(Pool *pool, const char *packageName) {
  // 1. Get the ID for the name (pass create=0 because we only want to lookup)
  Id name_id = pool_str2id(pool, packageName, 0);
  if (!name_id) {
    std::cout << "Package '" << packageName
              << "' does not exist in the pool.\n";
    return;
  }

  // 2. Prepare the pool for querying (builds the provider index)
  pool_createwhatprovides(pool);

  // 3. Iterate through every solvable that provides this name
  Id p, pp;
  FOR_PROVIDES(p, pp, name_id) {
    Solvable *s = pool_id2solvable(pool, p);

    // Print the package name and version
    std::cout << "Dependencies for " << pool_id2str(pool, s->name) << "="
              << pool_id2str(pool, s->evr) << ":\n";

    // 4. Iterate through the SOLVABLE_REQUIRES array
    if (s->requires) {
      Id req;
      // FIX 2: The internal array is named 'idarraydata'
      Id *reqp = s->repo->idarraydata + s->
                                          requires;

      // The array is 0-terminated
      while ((req = *reqp++) != 0) {
        // pool_dep2str handles both simple names ("foo") and relations ("foo
        // >= 1.0")
        std::cout << "  - " << pool_dep2str(pool, req) << "\n";
      }
    } else {
      std::cout << "  (none)\n";
    }
  }
}
