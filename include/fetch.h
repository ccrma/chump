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
using std::string;

namespace fs = std::filesystem;

// Fetch should download into a temp dir, not get a package object
// Return file object?
// I don't think installing needs to be broken out

// Abstract base class
class FetchBase {
public:
  virtual ~FetchBase() = default;
  virtual bool fetch(string data, string dir, Package package, fs::path temp_dir, string checksum) = 0;
  virtual bool fetch_manifest(string url, fs::path dir) = 0;
};


class Fetch : public FetchBase {
public:
  Fetch();
  Fetch(bool render_tui);
  // download a package file from a remote host
  bool fetch(string data, string dir, Package package, fs::path temp_dir, string checksum);
  // download the manifest from a remote host
  bool fetch_manifest(string url, fs::path dir);

public:
  bool isURL(string path);

private:
  static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
  fs::path chump_dir;
  bool render;
};


#endif
