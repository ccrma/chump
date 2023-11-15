// Package info and metadata
#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <string>
#include <vector>
#include <iostream>

// A Package describes the package spec.
struct Package {
  std::string owner;
  std::string name;
  std::string version;
  std::string api_version;
  std::string homepage;
  std::string repository;
  std::string specFile;
  std::vector<std::string> authors;
  std::string license;
  std::string description;
  std::vector<std::string> keywords;
  // Assets assets;

  // Equality operator overload
  bool operator==(const Package& other) const
    {
     return (owner == other.owner) &&
     (name == other.name) &&
     (version == other.version) &&
     (api_version == other.api_version) &&
     (homepage == other.homepage) &&
     (repository == other.repository) &&
     (specFile == other.specFile) &&
     (authors == other.authors) &&
     (license == other.license) &&
     (description == other.description) &&
     (keywords == other.keywords);
    }

  // Output stream operator overload
  friend std::ostream& operator<<(std::ostream& os, const Package& pkg)
  {
   os << "Owner: " << pkg.owner << "\n"
   << "Name: " << pkg.name << "\n"
   << "Version: " << pkg.version << "\n"
   << "API Version: " << pkg.api_version << "\n"
   << "Homepage: " << pkg.homepage << "\n"
   << "Repository: " << pkg.repository << "\n"
   << "Spec File: " << pkg.specFile << "\n"
   << "Authors: [";

   if (!pkg.authors.empty()) {
     os << pkg.authors[0];

     for (size_t i = 1; i < pkg.authors.size(); ++i) {
       os << ", " << pkg.authors[i];
     }
   }

   os << "]\nLicense: " << pkg.license << "\n"
      << "Description: " << pkg.description << "\n"
      << "Keywords: [";

   if (!pkg.keywords.empty()) {
     os << pkg.keywords[0];

     for (size_t i = 1; i < pkg.keywords.size(); ++i) {
       os << ", " << pkg.keywords[i];
     }
   }

   os << "]\n";
   return os;
  }
};

#endif
