#include <filesystem>
#include <iostream>

#include "fetch.h"

/*
  valid use cases:
  - chump info path/to/spec.json # directly link to the pkg file
  - chump info path/to/ # dir containing spec file
  - chump info www.package.com/ABSaturator.zip # the path is a url to be downloaded
  - chump info pkg-name # search pre-defined directories for the pkg name (or online pkg list)
 */
optional<Package> Fetch::fetch(std::string path) {
  if (isJSONFile(path)) {
    std::ifstream f(path);
    json data = json::parse(f);

    auto p = data.template get<Package>();
    return p;
  } else if (isDirectory(path)) {
    return {}; // TODO
  } else if (isURL(path)) {
    return {}; // TODO
  } else if (isPackageName(path)) {
    return {}; // TODO
  }

  
  // Handle other cases or return std::nullopt for unknown path types
  std::cerr << "Unknown path type: " << path << std::endl;
  return {};
}

bool Fetch::isJSONFile(std::string path) {
  // Check if the path has a json extension
  return (path.size() > 5) && (path.substr(path.size() - 5) == ".json");
}

bool Fetch::isDirectory(std::string path) {
  // Check if the path is a directory
  return std::filesystem::is_directory(path);
}

bool Fetch::isURL(std::string path) {
  // https://www.geeksforgeeks.org/check-if-an-url-is-valid-or-not-using-regular-expression/#
  const std::regex pattern("((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)");

  return regex_match(path, pattern);
}

bool Fetch::isPackageName(std::string path) {
  return path != "" && std::all_of(path.begin(), path.end(), ::isalnum);
}
