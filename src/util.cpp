#include "util.h"

#include "package_list.h"

#include <regex>

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <fstream>
#include <iostream>
#include <sstream>


#ifdef _WIN32
#include <windows.h>
#include <Shlobj.h>
#endif

using std::map;


// Returns the path to the directory where a package will be installed
// TODO expand to other OS
fs::path packagePath(Package p, fs::path install_dir) {
    fs::path package_dir = install_dir / p.name;
    return package_dir;
}

fs::path chumpDir() {
  fs::path home = getHomeDirectory();
#ifdef _WIN32
  fs::path chump_dir = home / "Documents" / "ChucK" / "packages";
#else
  fs::path chump_dir = home / ".chuck" / "packages";
#endif

  return chump_dir;
}

std::string whichOS() {
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
  return "windows";
#elif defined(__APPLE__)
  return "mac";
#elif defined(__linux__)
  return "linux";
#else
    std::cerr << "Unknown operating system" << std::endl;
    return "";
#endif
}

// Get user's home directory.
std::filesystem::path getHomeDirectory() {
    std::filesystem::path home_dir;

#ifdef _WIN32
    // Windows-specific code to get the user's home directory
    TCHAR szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, szPath))) {
        home_dir = szPath;
    } else {
        // If SHGetFolderPath fails, fallback to using environment variables
        const char* home_env = std::getenv("USERPROFILE");
        if (home_env != nullptr)
            home_dir = home_env;
        else
            throw std::runtime_error("Failed to get user's home directory");
    }
#else
    // Unix-like systems
    const char* home_env = std::getenv("HOME");
    if (home_env != nullptr)
        home_dir = home_env;
    else
        throw std::runtime_error("Failed to get user's home directory");
#endif

    return home_dir;
}

tuple<string, optional<string>> parsePackageName(string packageName) {
    std::regex pattern("(.*)=(.*)"); // Regular expression to match "name=version" or just "name"
    std::smatch matches;
    string name;
    optional<string> version;

    if (std::regex_match(packageName, matches, pattern)) {
      name = matches[1]; // The part before '='
      version = matches[2]; // The part after '='
    } else {
      name = packageName;
    }

    return {name, version};
}

// https://stackoverflow.com/questions/62503197/check-if-path-contains-another-in-c
bool is_subpath(const fs::path& path, const fs::path& base)
{
  if (path == base) return true; // edge case

  // Need to convert these paths to absolute or otherwise
  // there's inconsistencies between windows and unix.
  auto rel = fs::absolute(path).lexically_relative(fs::absolute(base));
  return !rel.empty() && rel.native()[0] != '.';
}


// Convert a filename to a sha 256 hash.
// https://docs.openssl.org/3.0/man3/EVP_DigestInit/#examples
// Do I know what's really going on here? Sort of...
std::string hash_file(fs::path filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
      throw std::runtime_error("Failed to open file: " + filename.string());
    }

    EVP_MD_CTX* md5Context = EVP_MD_CTX_new();
    EVP_MD_CTX_init(md5Context);
    EVP_DigestInit_ex(md5Context, EVP_sha256(), nullptr);

    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    while (!file.eof()) {
        file.read(buffer, bufferSize);
        EVP_DigestUpdate(md5Context, buffer, file.gcount());
    }

    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int hash_len;

    EVP_DigestFinal_ex(md5Context, result, &hash_len);
    file.close();

    EVP_MD_CTX_free(md5Context);

    std::stringstream ss;
    for(unsigned int i = 0; i < hash_len; ++i)
    {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }

    string hashed = ss.str();
    return hashed;
}

bool validate_manifest(fs::path manifest_path) {
  if (!fs::exists(manifest_path)) {
    std::cerr << "Unable to find manifest.json. try update the chump package list (`chump update -u`)" << std::endl;
    return false;
  }

  try {
    PackageList p(manifest_path);
  } catch (const std::exception &e) {
    std::cerr << "failed to validate manifest.json with the error: " << e.what() << std::endl;
    std::cerr << "try updating the manifest with `chump update -u`" << std::endl;
    return false;
  }

  return true;
}

// Convert a FileType enum to the appropriate dir to store it in inside a package dir
fs::path fileTypeToDir(FileType f) {
  fs::path empty;

  switch (f) {
    case PACKAGE_FILE:
      return empty;
    case DATA_FILE:
      return fs::path("_data");
    case EXAMPLE_FILE:
      return fs::path("_examples");
    case DOCS_FILE:
      return fs::path("_docs");
    case DEPS_FILE:
      return fs::path("_deps");
    default:
      return empty;
  }
}
