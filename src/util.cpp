#include "util.h"

// Returns the path to the directory where a package will be installed
fs::path packagePath(Package p) {
    const char* env_p = std::getenv("HOME");
    fs::path home = fs::path(env_p);
    fs::path package_dir = home / ".chuck/lib" / p.name;

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
