// package.cpp

#include "package.h"

// Equality operator overload
bool Package::operator==(const Package& other) const {
    return (name == other.name) &&
           (authors == other.authors) &&
           (homepage == other.homepage) &&
           (repository == other.repository) &&
           (license == other.license) &&
           (description == other.description) &&
           (keywords == other.keywords) &&
           (versions == other.versions);
}

bool PackageVersion::operator==(const PackageVersion& other) const {
    return (version == other.version) &&
           (api_version == other.api_version) &&
           (language_version == other.language_version) &&
           (os == other.os) &&
           (files == other.files);
}

// Output stream operator overload
std::ostream& operator<<(std::ostream& os, const Package& pkg) {
  os << "Name: " << pkg.name << "\n"
     << "Authors: [";
  if (!pkg.authors.empty()) {
    os << pkg.authors[0];

    for (size_t i = 1; i < pkg.authors.size(); ++i) {
      os << ", " << pkg.authors[i];
    }
  }
  os << "]\nHomepage: " << pkg.homepage << "\n"
     << "Repository: " << pkg.repository << "\n"
     << "License: " << pkg.license << "\n"
     << "Description: " << pkg.description << "\n"
     << "Keywords: [";

  if (!pkg.keywords.empty()) {
    os << pkg.keywords[0];

    for (size_t i = 1; i < pkg.keywords.size(); ++i) {
      os << ", " << pkg.keywords[i];
    }
  }

  os << "]\nVersions: [";
  if (!pkg.versions.empty()) {

    for (const auto& ver : pkg.versions) {
      os << ver << "\n";
    }
  }
  os << "]\n";
  return os;
}

std::ostream& operator<<(std::ostream& os, const PackageVersion& ver) {
  os << "Version: " << ver.version << "\n"
     << "API Version: " << ver.api_version << "\n"
     << "Language Version: " << ver.language_version << "\n"
     << "Operating System: " << ver.os << "\n"
     << "Files: [";

  if (!ver.files.empty()) {
    os << ver.files[0];

    for (size_t i = 1; i < ver.files.size(); ++i) {
      os << ", " << ver.files[i];
    }
  }
  os << "]\n";

  return os;
}

void to_json(json& j, const Package& p) {
  j = json{
    {"name", p.name},
    {"homepage", p.homepage},
    {"repository", p.repository},
    {"authors", p.authors},
    {"license", p.license},
    {"description", p.description},
    {"keywords", p.keywords},
    {"versions", p.versions}
    };
}

void from_json(const json& j, Package& p) {
    j.at("name").get_to(p.name);
    j.at("homepage").get_to(p.homepage);
    j.at("repository").get_to(p.repository);
    j.at("authors").get_to(p.authors);
    j.at("license").get_to(p.license);
    j.at("description").get_to(p.description);
    j.at("keywords").get_to(p.keywords);
    j.at("versions").get_to(p.versions);
}

void to_json(json& j, const PackageVersion& p) {
  j = json{
        {"version", p.version},
        {"api_version", p.api_version},
        {"language_version", p.language_version},
        {"os", p.os},
        {"files", p.files},
    };
}

void from_json(const json& j, PackageVersion& p) {
    j.at("version").get_to(p.version);
    j.at("api_version").get_to(p.api_version);
    j.at("language_version").get_to(p.language_version);
    j.at("os").get_to(p.os);
    j.at("files").get_to(p.files);
}

optional<PackageVersion> Package::latest_version(string os) {
  optional<PackageVersion> latest;

  for (PackageVersion ver : versions) {
    if (ver.os != os) continue;
  }
  return {};
}

// converts a string of the format "1.2.3" into a Version struct
Version parseVersionString(const std::string& versionStr) {
    Version version;

    vector<int> values;

    size_t pos = 0;
    string token;
    string delimiter = ".";

    /*****/
    size_t last = 0;
    size_t next = 0;
    while ((next = versionStr.find(delimiter, last)) != string::npos) {
      token = versionStr.substr(last, next-last);

      int value = std::stoi(token);
      values.push_back(value);

      last = next + 1;
    }

    token = versionStr.substr(last, versionStr.size() - last);
    int value = std::stoi(token);
    values.push_back(value);

    if (values.size() != 3) {
      throw std::invalid_argument("Invalid version string format");
    }

    version.major = values[0];
    version.minor = values[1];
    version.patch = values[2];

    return version;
}

bool operator==(const Version& lhs, const Version& rhs) {
  return std::tie(lhs.major, lhs.minor, lhs.patch) == std::tie(rhs.major, rhs.minor, rhs.patch);
}

bool operator!=(const Version& lhs, const Version& rhs) {
  return std::tie(lhs.major, lhs.minor, lhs.patch) != std::tie(rhs.major, rhs.minor, rhs.patch);
}

bool operator<(const Version& lhs, const Version& rhs) {
  return std::tie(lhs.major, lhs.minor, lhs.patch) < std::tie(rhs.major, rhs.minor, rhs.patch);
}

bool operator<=(const Version& lhs, const Version& rhs) {
  return std::tie(lhs.major, lhs.minor, lhs.patch) <= std::tie(rhs.major, rhs.minor, rhs.patch);
}

bool operator>(const Version& lhs, const Version& rhs) {
  return std::tie(lhs.major, lhs.minor, lhs.patch) > std::tie(rhs.major, rhs.minor, rhs.patch);
}

bool operator>=(const Version& lhs, const Version& rhs) {
  return std::tie(lhs.major, lhs.minor, lhs.patch) >= std::tie(rhs.major, rhs.minor, rhs.patch);
}
