#include "util.h"

// Returns the path to the directory where a package will be installed
fs::path packagePath(Package p) {
    const char* env_p = std::getenv("HOME");
    fs::path home = fs::path(env_p);
    fs::path package_dir = home / ".chuck/lib" / p.name;

    return package_dir;
}
