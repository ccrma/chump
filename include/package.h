// Package info and metadata
#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <string>
#include <vector>
#include <iostream>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using std::string;
using std::map;
using std::vector;
using std::optional;

struct PackageVersion {
  // Version follows the major.minor.patch versioning scheme. Will enforce this strictly
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

// A Package describes the package spec.
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
  optional<PackageVersion> latest_version(string os);
};

// Function declarations for JSON serialization/deserialization
void to_json(json& j, const Package& p);
void from_json(const json& j, Package& p);

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
