#include <filesystem>
#include <iostream>

#include "fetch.h"

Package Fetch::fetch(std::string path) {
  Package p;
  // Check if the file path exists
  std::cout << path << std::endl;
  if (std::filesystem::exists(path)) {
    std::ifstream f(path);
    json data = json::parse(f);

    p = data.template get<Package>();

  } else {
    std::cout << "The file path is not valid or the file doesn't exist." << std::endl;
  }

  // only support filepath for now
  return p;
}
