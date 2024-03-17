// Package info and metadata
#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <string>
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using std::string;
using std::map;
using std::vector;

// A Package describes the package spec.
struct Package {
  string owner;
  string name;
  string version;
  string api_version;
  map<string, vector<string>> files; // map of operating system to list of files to download
  string homepage;
  string repository;
  string specFile;
  vector<string> authors;
  string license;
  string description;
  vector<string> keywords;

  // Equality operator overload
  bool operator==(const Package& other) const;

  // Output stream operator overload
  friend std::ostream& operator<<(std::ostream& os, const Package& pkg);
};

// Function declarations for JSON serialization/deserialization
void to_json(json& j, const Package& p);
void from_json(const json& j, Package& p);

#endif
