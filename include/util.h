#ifndef __UTIL_H__
#define __UTIL_H__

#include "package.h"

using std::optional;
using std::string;
using std::tuple;

namespace fs = std::filesystem;

fs::path packagePath(Package p, fs::path install_dir);

string whichOS();

fs::path getHomeDirectory();

fs::path chumpDir();

tuple<string, optional<string>> parsePackageName(string packageName);

#endif
