// Package info and metadata
#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <string>
#include <vector>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// A Package describes the package spec.
struct Package {
  std::string owner;
  std::string name;
  std::string version;
  std::string api_version;
  std::vector<std::string> files; // download urls
  std::string homepage;
  std::string repository;
  std::string specFile;
  std::vector<std::string> authors;
  std::string license;
  std::string description;
  std::vector<std::string> keywords;

  // Equality operator overload
  bool operator==(const Package& other) const;

  // Output stream operator overload
  friend std::ostream& operator<<(std::ostream& os, const Package& pkg);
};

// Function declarations for JSON serialization/deserialization
void to_json(json& j, const Package& p);
void from_json(const json& j, Package& p);

#endif
