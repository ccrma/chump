//-----------------------------------------------------------------------------
// name: package.h
// desc: Package information and version metadata
//-----------------------------------------------------------------------------

#ifndef __PACKAGE_H__
i#define __PACKAGE_H__

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

//-----------------------------------------------------------------------------
// PackageVersion describes a specific version of a package. For examples,
// this is a usually a .chug file, the associated download link, and
// metadata specifying which language and API versions it is compatible with.
// -----------------------------------------------------------------------------
struct PackageVersion {
  string version;
  string api_version;
  string language_version;
  string os;
  vector<string> files;

  // Equality operator overload
  bool operator==(const PackageVersion& other) const;

  // Output stream operator overload
  friend std::ostream& operator<<(std::ostream& os, const PackageVersion& pkg);
};

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
};

// Function declarations for JSON serialization/deserialization
void to_json(json& j, const Package& p);
void from_json(const json& j, Package& p);


//-----------------------------------------------------------------------------
// Version is a struct describing semantic versioning. It follows the
// major.minor.patch versioning scheme. i.e. "1.2.1"
// -----------------------------------------------------------------------------

struct Version {
  int major;
  int minor;
  int patch;

  // Equality operator overload
  friend bool operator==(const Version& lhs, const Version& rhs);
  friend bool operator!=(const Version& lhs, const Version& rhs);
  friend bool operator<(const Version& lhs, const Version& rhs);
  friend bool operator<=(const Version& lhs, const Version& rhs);
  friend bool operator>(const Version& lhs, const Version& rhs);
  friend bool operator>=(const Version& lhs, const Version& rhs);
};

Version parseVersionString(const std::string& versionStr);

#endif
