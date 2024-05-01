#ifndef __UTIL_H__
#define __UTIL_H__

#include "package.h"

namespace fs = std::filesystem;

fs::path packagePath(Package p);

std::string whichOS();

fs::path getHomeDirectory();

fs::path chumpDir();

#endif
