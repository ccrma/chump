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

bool is_subpath(const fs::path& path, const fs::path& base);

std::string hash_file(fs::path filename);

// validate that the manifest.json is able to be loaded correctly
bool validate_manifest(fs::path manifest_path);

fs::path fileTypeToDir(FileType f);

#endif
