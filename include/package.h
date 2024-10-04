//-----------------------------------------------------------------------------
// name: package.h
// desc: Package information and version metadata
//-----------------------------------------------------------------------------

#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include <nlohmann/json.hpp>

#include "chuck_version.h"

using json = nlohmann::json;
using std::string;
using std::map;
using std::vector;
using std::optional;
using std::tuple;

struct PackageVersion;

// Function declarations for JSON serialization/deserialization
void to_json(json& j, const PackageVersion& p);
void from_json(const json& j, PackageVersion& p);


//-----------------------------------------------------------------------------
// Package describes a package spec. This will include metadata
// (authors, description, etc), as well as a list of all available
// verisons of this package.
// -----------------------------------------------------------------------------
struct Package {
  string name;
  vector<string> authors;
  string homepage;
  string repository;
  string license;
  string description;
  vector<string> keywords;

  vector<PackageVersion> versions;

  // Equality operator overload
  bool operator==(const Package& other) const;

  // Output stream operator overload
  friend std::ostream& operator<<(std::ostream& os, const Package& pkg);

  // Automatically find highest version package compatible with your system.
  optional<PackageVersion> latest_version(string os, ChuckVersion language_ver, ApiVersion api_ver);

  optional<PackageVersion> version(PackageVersion ver, string os, ChuckVersion language_ver, ApiVersion api_ver);
};

// Function declarations for JSON serialization/deserialization
void to_json(json& j, const Package& p);
void from_json(const json& j, Package& p);


//-----------------------------------------------------------------------------
// PackageVersion describes a specific version of a package. For examples,
// this is a usually a .chug file, the associated download link, and
// metadata specifying which language and API versions it is compatible with.
//
// PackageVersion follows the major.minor.patch versioning scheme. i.e. "1.2.1"
// -----------------------------------------------------------------------------
struct PackageVersion {
  PackageVersion();
  PackageVersion(string version);
  PackageVersion(int major, int minor, int patch);
  PackageVersion(string version, string language_version_min,
                 string api_version, string os, vector<tuple<string,string>> files);
  PackageVersion(string version, string language_version_min,
                 string language_version_max, string api_version,
                 string os, vector<tuple<string,string>> files);

  int major, minor, patch;

  string api_version;
  // minimum compatible version of chuck
  string language_version_min;
  // Maximal compatible version of chuck. If this is None, then
  // all versions >= language_version_min are compatible
  optional<string> language_version_max;
  string os;
  vector<tuple<string,string>> files;

  // Equality operator overload
  bool operator==(const PackageVersion& other) const;
  bool operator!=(const PackageVersion& other) const;
  bool operator<(const PackageVersion& other) const;
  bool operator<=(const PackageVersion& other) const;
  bool operator>(const PackageVersion& other) const;
  bool operator>=(const PackageVersion& other) const;

  // Output stream operator overload
  friend std::ostream& operator<<(std::ostream& os, const PackageVersion& pkg);

  void setVersionString(string version);
  string getVersionString() const;
};

#endif
