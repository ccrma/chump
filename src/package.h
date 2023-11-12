// Package info and metadata

// A Package describes the package spec.
struct Package {
  std::string owner;
  std::string name;
  std::string version;
  std::string api_version;
  std::string homepage;
  std::string repository;
  std::string specFile
  std::vector<std::string> authors;
  std::string license;
  std::string description;
  std::vector<std::string> keywords;
  // Assets assets;
}
