#include <filesystem>
#include <iostream>

#include "fetch.h"


// Fetch::Fetch() {
// }


Package Fetch::fetch(std::string path) {
  Package p;
  // Check if the file path exists
  if (std::filesystem::exists(path)) {
    std::cout << "The file path is valid." << std::endl;
  } else {
    std::cout << "The file path is not valid or the file doesn't exist." << std::endl;
  }
  
  // only support filepath for now
  return p;
}
