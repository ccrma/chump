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

namespace fs = std::filesystem;

struct PackageVersion;
struct File;

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
    optional<PackageVersion> latest_version(string os);

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
                   string api_version, string os, vector<File> files);
    PackageVersion(string version, string language_version_min,
                   string os, vector<File> files);
    PackageVersion(string version, string language_version_min,
                   string language_version_max, string api_version,
                   string os, vector<File> files);

    int major, minor, patch;

    // Compatible chugin api version. If this is None, then there are no
    // chugins being used in this pacakge and this doesn't matter.
    optional<ApiVersion> api_version;
    // minimum compatible version of chuck
    ChuckVersion language_version_min;
    // Maximal compatible version of chuck. If this is None, then
    // all versions >= language_version_min are compatible
    optional<ChuckVersion> language_version_max;
    string os;
    vector<File> files;

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

//-----------------------------------------------------------------------------
// InstalledVersion is a description of a package that gets stored as a json
//   file inside an installed package. It includes metadata, and a version
//   definition that allows chump to properly update packages.
// -----------------------------------------------------------------------------
struct InstalledVersion {
    InstalledVersion();
    // files will need to be added separately
    InstalledVersion(Package pkg, PackageVersion v);

    // Export a PackageVersion
    PackageVersion version();
    // Export a Package
    Package package();
    string name;

    vector<string> authors;
    string homepage;
    string repository;
    string license;
    string description;
    vector<string> keywords;

    int major, minor, patch;

    // Compatible chugin api version. If this is None, then there are no
    // chugins being used in this pacakge and this doesn't matter.
    optional<ApiVersion> api_version;
    // minimum compatible version of chuck
    ChuckVersion language_version_min;
    // Maximal compatible version of chuck. If this is None, then
    // all versions >= language_version_min are compatible
    optional<ChuckVersion> language_version_max;
    string os;
    vector<fs::path> files;
};

// Function declarations for JSON serialization/deserialization
void to_json(json& j, const InstalledVersion& p);
void from_json(const json& j, InstalledVersion& p);

// What kind of file is this? Package file, dep, example, etc...
enum FileType {
    PACKAGE_FILE,
    DATA_FILE,
    EXAMPLE_FILE,
    DOCS_FILE,
    DEPS_FILE,
    ZIP_FILE // Typically the entire package will be zipped and this will be the only file
};

static const map<string, FileType> stringToFiletype = {
    {"package", PACKAGE_FILE},
    {"data", DATA_FILE},
    {"example", EXAMPLE_FILE},
    {"docs", DOCS_FILE},
    {"deps", DEPS_FILE},
    {"zip", ZIP_FILE}
};

static const map<FileType, string> filetypeToString = {
    {PACKAGE_FILE, "package"},
    {DATA_FILE, "data"},
    {EXAMPLE_FILE, "example"},
    {DOCS_FILE, "docs"},
    {DEPS_FILE, "deps"},
    {ZIP_FILE, "zip"}
};

//-----------------------------------------------------------------------------
// File provides metadata needed to download and install a file for chump:
// url, install path, and checksum
// -----------------------------------------------------------------------------
struct File {
    string url;
    fs::path local_dir;
    FileType file_type;
    string checksum;
};

// Function declarations for JSON serialization/deserialization
void to_json(json& j, const File& f);
void from_json(const json& j, File& f);

#endif
