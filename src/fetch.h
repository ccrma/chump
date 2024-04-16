#ifndef __FETCH_H__
#define __FETCH_H__

#include "package.h"

#include <nlohmann/json.hpp>
#include <curl/curl.h>

#include <fstream>
#include <regex>
#include <cstdlib>
#include <optional>

using json = nlohmann::json;

using std::optional;

namespace fs = std::filesystem;

// Fetch should download into a temp dir, not get a package object
// Return file object?
// I don't think installing needs to be broken out

// Abstract base class
class FetchBase {
public:
  virtual ~FetchBase() = default;
  virtual optional<Package> fetch(std::string data, Package package) = 0;
};


class Fetch : public FetchBase {
 public:
  optional<Package> fetch(std::string data, Package package);

public:
  bool isJSONFile(std::string path);
  bool isDirectory(std::string path);
  bool isURL(std::string path);
  bool isPackageName(std::string path);

private:
  static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
};


#endif
