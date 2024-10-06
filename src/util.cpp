#include "util.h"

#include <regex>

#ifdef _WIN32
#include <windows.h>
#include <Shlobj.h>
#endif


// Returns the path to the directory where a package will be installed
// TODO expand to other OS
fs::path packagePath(Package p, fs::path install_dir) {
    fs::path package_dir = install_dir / p.name;
    return package_dir;
}

fs::path chumpDir() {
  fs::path home = getHomeDirectory();
#ifdef _WIN32
  fs::path chump_dir = home / "Documents" / "chuck" / "chugins" / ".chump";
#else
  fs::path chump_dir = home / ".chuck" / "lib" / ".chump";
#endif

  return chump_dir;
}

std::string whichOS() {
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
  return "windows";
#elif defined(__APPLE__)
  return "mac";
#elif defined(__linux__)
  return "linux";
#else
    std::cerr << "Unknown operating system" << std::endl;
    return "";
#endif
}

// Get user's home directory.
std::filesystem::path getHomeDirectory() {
    std::filesystem::path home_dir;

#ifdef _WIN32
    // Windows-specific code to get the user's home directory
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, szPath))) {
        home_dir = szPath;
    } else {
        // If SHGetFolderPath fails, fallback to using environment variables
        const char* home_env = std::getenv("USERPROFILE");
        if (home_env != nullptr)
            home_dir = home_env;
        else
            throw std::runtime_error("Failed to get user's home directory");
    }
#else
    // Unix-like systems
    const char* home_env = std::getenv("HOME");
    if (home_env != nullptr)
        home_dir = home_env;
    else
        throw std::runtime_error("Failed to get user's home directory");
#endif

    return home_dir;
}

tuple<string, optional<string>> parsePackageName(string packageName) {
    std::regex pattern("(.*)=(.*)"); // Regular expression to match "name=version" or just "name"
    std::smatch matches;
    string name;
    optional<string> version;

    if (std::regex_match(packageName, matches, pattern)) {
      name = matches[1]; // The part before '='
      version = matches[2]; // The part after '='
    } else {
      name = packageName;
    }

    return {name, version};
}

// https://stackoverflow.com/questions/62503197/check-if-path-contains-another-in-c
bool is_subpath(const fs::path& path, const fs::path& base)
{
  // Need to convert these paths to absolute or otherwise 
  // there's inconsistencies between windows and unix.
  auto rel = fs::absolute(path).lexically_relative(fs::absolute(base));
  return !rel.empty() && rel.native()[0] != '.';
}
