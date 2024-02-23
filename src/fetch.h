#ifndef __FETCH_H__
#define __FETCH_H__

#include "package.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <regex>

using json = nlohmann::json;

using std::optional;

// Fetch should download into a temp dir, not get a package object
// Return file object?
// I don't think installing needs to be broken out

// Abstract base class
class FetchBase {
public:
  virtual ~FetchBase() = default;
  virtual optional<Package> fetch(std::string data) = 0;
};


class Fetch : public FetchBase {
 public:
  optional<Package> fetch(std::string data);

public:
  bool isJSONFile(std::string path);
  bool isDirectory(std::string path);
  bool isURL(std::string path);
  bool isPackageName(std::string path);
};


#endif
