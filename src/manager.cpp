
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
        std::cerr << "[chump]: package " << packageName << " not found." << std::endl;
        return false;
    }

    Package package = pkg.value();

    optional<PackageVersion> ver;
    if (version_string) {
        try {
            PackageVersion pkgver(version_string.value());
            ver = package.version(pkgver, os, language_version, api_version);
        } catch (std::invalid_argument& e) {
            std::cerr << "[chump]: " << e.what() << '\n';
            return false;
        }
    } else {
        ver = package.latest_version(os, language_version, api_version);
    }

    if (!ver) {
        std::cerr << "[chump]: unable to find version of package " << package.name
                  << " that works on your system" << std::endl;

        return false;
    }

    PackageVersion version = ver.value();

    // if there is already a packages/PACKAGE directory, error out and tell the user to call update
    fs::path install_dir = packagePath(package, chump_dir);

    if (fs::exists(install_dir / "version.json")) {
        std::cerr << "[chump]: the package '" << package.name << "' already exists." << std::endl;
        std::cerr << "[chump]: use `chump update " << package.name << "' to update the existing package" << std::endl;
        std::cerr << "[chump]: or use `chump uninstall " << package.name << "` to remove the package" << std::endl;
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
        FileType file_type = file.file_type;

        bool result = fetch->fetch(url, dir, package, temp_dir, file_type, checksum);
        if (!result) {
            std::cerr << "[chump]: failed to fetch " << url << ", exiting." << std::endl;
            return false;
        }
    }

    // If any of our fetched files are zip files, unzip them and then
    // delete the original zip file so it won't be copied over to the
    // packages dir.
    for (auto file: version.files) {
        if (file.file_type != ZIP_FILE) continue;

        fs::path filename = fs::path(file.url).filename();
        fs::path dir = temp_dir / file.local_dir;

        // Extract the file
        if (!unzipFile((dir / filename).string(), dir.string())) return false;

        fs::remove(dir / filename);
    }

    // Add all files to the InstalledVersion file list
    InstalledVersion installed_ver = InstalledVersion(package, version);
    for (auto const& dir_entry : fs::recursive_directory_iterator(temp_dir)) {
        if (fs::is_regular_file(dir_entry)) {
            installed_ver.files.push_back(fs::relative(dir_entry, temp_dir));
        }
    }


    // create install dir if needed
    fs::create_directory(install_dir);

    // Copy temp files over to the install directory
    try {
        fs::copy(temp_dir, install_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    } catch (std::filesystem::filesystem_error& e) {
        std::cerr << "[chump]: " << e.what() << '\n';
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

bool Manager::install_local(fs::path pkgDefn, fs::path pkgVer, fs::path pkgZip) {
    if (!fs::is_regular_file(pkgDefn)) {
        std::cerr << "[chump]: package definition " << pkgDefn << "is not a file, exiting..." << std::endl;
        return false;
    }

    if (!fs::is_regular_file(pkgVer)) {
        std::cerr << "[chump]: package version " << pkgVer << "is not a file, exiting..." << std::endl;
        return false;
    }

    if (!fs::is_regular_file(pkgZip)) {
        std::cerr << "[chump]: package zip " << pkgZip << "is not a file, exiting..." << std::endl;
        return false;
    }

    // only supporting zip for now
    if (pkgZip.extension() != ".zip") {
        std::cerr << "[chump]: package path " << pkgZip << "is not a zip file, exiting..." << std::endl;
    }

    // parse package and version files
    std::ifstream ver_stream(pkgVer);
    if (!ver_stream.good()) {
        std::cerr << "[chump]: unable to open " << pkgVer << std::endl;
        ver_stream.close();
        return false;
    }

    json pkg_ver;
    // need to close file on failure
    try {
        pkg_ver = json::parse(ver_stream);
        ver_stream.close();
    } catch (const std::exception &e) {
        ver_stream.close();
        throw;
    }

    PackageVersion version = pkg_ver.template get<PackageVersion>();

    std::ifstream pkg_stream(pkgDefn);
    if (!pkg_stream.good()) {
        std::cerr << "[chump]: unable to open " << pkgDefn << std::endl;
        pkg_stream.close();
        return false;
    }

    json pkg_json;
    // need to close file on failure
    try {
        pkg_json = json::parse(pkg_stream);
        pkg_stream.close();
    } catch (const std::exception &e) {
        pkg_stream.close();
        throw;
    }

    Package package = pkg_json.template get<Package>();

    // if there is already a packages/PACKAGE directory, error out and tell the user to call update
    fs::path install_dir = packagePath(package, chump_dir);

    if (fs::exists(install_dir / "version.json")) {
        std::cerr << "[chump]: the package '" << package.name << "' already exists." << std::endl;
        std::cerr << "[chump]: use `chump update " << package.name << "' to update the existing package" << std::endl;
        std::cerr << "[chump]: or use `chump uninstall " << package.name << "` to remove the package" << std::endl;
        return false;
    }

    fs::create_directory(install_dir);
    // Unzip the local file to the installed directory
    if (!unzipFile(pkgZip.string(), install_dir.string())) return false;


    // Add all files to the InstalledVersion file list
    InstalledVersion installed_ver = InstalledVersion(package, version);
    for (auto const& dir_entry : fs::recursive_directory_iterator(install_dir)) {
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
        std::cerr << "[chump]: package " << packageName << " not found." << std::endl;
        return false;
    }

    Package package = pkg.value();

    // if there is already a .chump/PACKAGE directory, error out and tell the user to call update
    fs::path install_dir = packagePath(package, chump_dir);

    if (!fs::exists(install_dir)) {
        std::cerr << "[chump]: the install directory '" << install_dir << "' does not exist." << std::endl;
        std::cerr << "[chump]: use `chump install " << package.name << "' to install the existing package" << std::endl;
        return false;
    }

    // Validate that the version is newer
    std::ifstream f(install_dir / "version.json");

    if (!f.good()) {
        std::cerr << "[chump]: unable to open " << install_dir / "version.json" << std::endl;
        f.close();
        return false;
    }

    json pkg_ver = json::parse(f);
    f.close();
    InstalledVersion installed_version = pkg_ver.template get<InstalledVersion>();
    PackageVersion curr_version = installed_version.version();

    optional<PackageVersion> ver = package.latest_version(os, language_version, api_version);

    if (!ver) {
        std::cerr << "[chump]: unable to find version of package " << package.name
                  << " that works on your system" << std::endl;

        return false;
    }

    PackageVersion latest_version = ver.value();

    if (curr_version == latest_version) {
        std::cerr << "[chump]: " << package.name << " is already up-to-date." << std::endl;
        return true;
    }

    if (curr_version > latest_version) {
        std::cerr << "[chump]: " << package.name << " is installed already with a newer version." << std::endl;
        return true;
    }

    for (auto file: installed_version.files) {
        fs::path curr_dir = (install_dir / file.parent_path()).lexically_normal();
        fs::path curr_file = (install_dir / file).lexically_normal();

        fs::remove(curr_file);
        if (fs::is_empty(curr_dir)) {
            fs::remove(curr_dir);
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
        FileType file_type = file.file_type;

        bool result = fetch->fetch(url, dir, package, temp_dir, file_type, checksum);
        if (!result) {
            std::cerr << "[chump]: failed to fetch " << url << ", exiting." << std::endl;
            return false;
        }
    }

    // If any of our fetched files are zip files, unzip them and then
    // delete the original zip file so it won't be copied over to the
    // packages dir.
    for (auto file: latest_version.files) {
        if (file.file_type != ZIP_FILE) continue;

        fs::path filename = fs::path(file.url).filename();
        fs::path dir = temp_dir / file.local_dir;

        // Extract the file
        if (!unzipFile((dir / filename).string(), dir.string())) return false;

        fs::remove(dir / filename);
    }

    InstalledVersion installed_ver(package, latest_version);
    for (auto const& dir_entry : fs::recursive_directory_iterator(temp_dir)) {
        if (fs::is_regular_file(dir_entry)) {
            installed_ver.files.push_back(fs::relative(dir_entry, temp_dir));
        }
    }

    // create install dir if needed
    fs::create_directory(install_dir);

    // Copy temp files over to the install directory
    try {
        fs::copy(temp_dir, install_dir, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
    } catch (fs::filesystem_error& e) {
        std::cerr << "[chump]: " << e.what() << '\n';
        return false;
    }


    // Write version.json to file.
    json latest_version_json = installed_ver;
    std::ofstream o(install_dir / "version.json");
    o << std::setw(4) << latest_version_json << std::endl;
    o.close();

    return true;
}

bool Manager::uninstall(string packageName) {
    // if there isn't already a .chump/PACKAGE directory, error out and tell the user to install
    fs::path packageNamePath = fs::path(packageName);
    fs::path install_dir = chump_dir / packageNamePath;

    if (!fs::exists(install_dir)) {
        std::cerr << "[chump]: the install directory '" << install_dir << "' does not exist." << std::endl;
        std::cerr << "[chump]: use `chump install " << packageName << "' to install the existing package" << std::endl;
        return false;
    }

    // Validate that the version file exists
    std::ifstream f(install_dir / "version.json");

    if (!f.good()) {
        std::cerr << "[chump]: unable to open " << install_dir / "version.json" << std::endl;
        return false;
    }

    json pkg_ver;
    try {
        pkg_ver = json::parse(f);
    } catch (const std::exception &e) {
        f.close();
        throw;
    }

    f.close();
    InstalledVersion installed_ver = pkg_ver.template get<InstalledVersion>();
    PackageVersion curr_ver = installed_ver.version();

    // Remove all files associated with package
    for (auto file: installed_ver.files) {
        fs::path filepath = file.lexically_normal();
        std::cout << "Removing " << install_dir / filepath << std::endl;
        fs::remove(install_dir / filepath);

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

std::vector<Package> Manager::listPackages() {
    std::vector<Package> pkg_list = package_list->get_packages();

    for (auto const& dir_entry : fs::directory_iterator{chump_dir}) {
        if (!fs::is_directory(dir_entry)) continue;
        if (!fs::exists(dir_entry.path() / "version.json")) continue;

        optional<InstalledVersion> installed_version = getInstalledVersion(dir_entry);

        if (!installed_version) continue;

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
        std::cerr << "[chump]: failed to fetch manifest.json, exiting." << std::endl;
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

    temp_manifest.close();
    curr_manifest.close();

    if (are_equal) {
        std::cerr << "[chump]: current manifest is up-to-date, doing nothing" << std::endl;
        return false;
    }

    // Copy temp files over to the install directory
    try {
        fs::remove(chump_dir / "manifest.json");
        fs::copy(temp_dir / "manifest.json", chump_dir / "manifest.json",
                 fs::copy_options::overwrite_existing);
    } catch (std::filesystem::filesystem_error& e) {
        std::cerr << "[chump]: " << e.what() << '\n';
        return false;
    }

    std::cerr << "[chump]: manifest.json was successfully updated!" << std::endl;
    return true;
}
