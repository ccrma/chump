// package.cpp

#include "package.h"

// Equality operator overload
bool Package::operator==(const Package& other) const {
    return (owner == other.owner) &&
           (name == other.name) &&
           (version == other.version) &&
           (api_version == other.api_version) &&
           (homepage == other.homepage) &&
           (repository == other.repository) &&
           (files == other.files) &&
           (specFile == other.specFile) &&
           (authors == other.authors) &&
           (license == other.license) &&
           (description == other.description) &&
           (keywords == other.keywords);
}

// Output stream operator overload
std::ostream& operator<<(std::ostream& os, const Package& pkg) {
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

    os << "]\nFiles: [";
    if (!pkg.files.empty()) {
        os << pkg.files[0];

        for (size_t i = 1; i < pkg.files.size(); ++i) {
            os << ", " << pkg.files[i];
        }
    }
    os << "]\n";
    return os;
}

void to_json(json& j, const Package& p) {
  j = json{
        {"owner", p.owner},
        {"name", p.name},
        {"version", p.version},
        {"api_version", p.api_version},
        {"files", p.files},
        {"homepage", p.homepage},
        {"repository", p.repository},
        {"spec_file", p.specFile},
        {"authors", p.authors},
        {"license", p.license},
        {"description", p.description},
        {"keywords", p.keywords}
    };
}

void from_json(const json& j, Package& p) {
    j.at("owner").get_to(p.owner);
    j.at("name").get_to(p.name);
    j.at("version").get_to(p.version);
    j.at("api_version").get_to(p.api_version);
    j.at("files").get_to(p.files);
    j.at("homepage").get_to(p.homepage);
    j.at("repository").get_to(p.repository);
    j.at("spec_file").get_to(p.specFile);
    j.at("authors").get_to(p.authors);
    j.at("license").get_to(p.license);
    j.at("description").get_to(p.description);
    j.at("keywords").get_to(p.keywords);
}
