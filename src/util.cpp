#include "util.h"

#ifdef _WIN32
#include <windows.h>
#include <Shlobj.h>
#endif


// Returns the path to the directory where a package will be installed
// TODO expand to other OS
fs::path packagePath(Package p) {
    const char* env_p = std::getenv("HOME");
    fs::path home = fs::path(env_p);
    fs::path package_dir = home / ".chuck/lib/.chump" / p.name;

    return package_dir;
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
std::filesystem::path get_home_directory() {
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
