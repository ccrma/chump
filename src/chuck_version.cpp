
#include "chuck_version.h"

#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <regex>


ChuckVersion::ChuckVersion() {

}

ChuckVersion::ChuckVersion(string version_string) {
  set_version(version_string);
}

cdChuckVersion ChuckVersion::makeSystemVersion() {
  // Query chuck to get verison string
#ifdef _WIN32
  FILE* pipe = _popen("chuck --query:version 2>&1", "r");
#else
  FILE* pipe = popen("chuck --query:version 2>&1", "r");
#endif
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }

  std::string version;

  constexpr int buffer_size = 128;
  char buffer[buffer_size];
  while (fgets(buffer, buffer_size, pipe) != nullptr) {
    version += buffer;
  }

#ifdef _WIN32
  auto status = _pclose(pipe);
#else
  auto status = pclose(pipe);
#endif
  if (status == -1) {
    throw std::runtime_error("pclose() failed!");
  }

  return ChuckVersion(version);
}

ChuckVersion ChuckVersion::makeVersion(string version_string) {
  return ChuckVersion(version_string);
}


void ChuckVersion::set_version(string version_string) {
  std::regex version_regex("(\\d+)\\.(\\d+)\\.(\\d+)\\.(\\d+)");
  std::smatch match;

  if (std::regex_search(version_string, match, version_regex)) {
    mega = std::stoi(match[1]);
    major = std::stoi(match[2]);
    minor = std::stoi(match[3]);
    patch = std::stoi(match[4]);
  } else {
    throw std::runtime_error("unable to construct version string from " + version_string);
  }
}

bool operator==(const ChuckVersion& lhs, const ChuckVersion& rhs) {
  return std::tie(lhs.mega, lhs.major, lhs.minor, lhs.patch) == std::tie(rhs.mega, rhs.major, rhs.minor, rhs.patch);
}

bool operator!=(const ChuckVersion& lhs, const ChuckVersion& rhs) {
  return std::tie(lhs.mega, lhs.major, lhs.minor, lhs.patch) != std::tie(rhs.mega, rhs.major, rhs.minor, rhs.patch);
}

bool operator<(const ChuckVersion& lhs, const ChuckVersion& rhs) {
  return std::tie(lhs.mega, lhs.major, lhs.minor, lhs.patch) < std::tie(rhs.mega, rhs.major, rhs.minor, rhs.patch);
}

bool operator<=(const ChuckVersion& lhs, const ChuckVersion& rhs) {
  return std::tie(lhs.mega, lhs.major, lhs.minor, lhs.patch) <= std::tie(rhs.mega, rhs.major, rhs.minor, rhs.patch);
}

bool operator>(const ChuckVersion& lhs, const ChuckVersion& rhs) {
  return std::tie(lhs.mega, lhs.major, lhs.minor, lhs.patch) > std::tie(rhs.mega, rhs.major, rhs.minor, rhs.patch);
}

bool operator>=(const ChuckVersion& lhs, const ChuckVersion& rhs) {
  return std::tie(lhs.mega, lhs.major, lhs.minor, lhs.patch) >= std::tie(rhs.mega, rhs.major, rhs.minor, rhs.patch);
}

// Output stream operator overload
std::ostream& operator<<(std::ostream& os, const ChuckVersion& ver) {
  os << ver.mega << "." << ver.major << "." << ver.minor << "." << ver.patch;

  return os;
}



ApiVersion::ApiVersion() {
}

ApiVersion::ApiVersion(string version_string) {
  set_version(version_string);
}

ApiVersion ApiVersion::makeSystemVersion() {
  // Query chuck to get api version string
#ifdef _WIN32
  FILE* pipe = _popen("chuck --query:version_chugin_api 2>&1", "r");
#else
  FILE* pipe = popen("chuck --query:version_chugin_api 2>&1", "r");
#endif
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }

  std::string version;

  constexpr int buffer_size = 128;
  char buffer[buffer_size];
  while (fgets(buffer, buffer_size, pipe) != nullptr) {
    version += buffer;
  }

#ifdef _WIN32
  auto status = _pclose(pipe);
#else
  auto status = pclose(pipe);
#endif
  if (status == -1) {
    throw std::runtime_error("pclose() failed!");
  }

  return ApiVersion(version);
}

ApiVersion ApiVersion::makeVersion(string version_string) {
  return ApiVersion(version_string);
}

void ApiVersion::set_version(string version_string) {
  std::regex version_regex("(\\d+)\\.(\\d+)\\s*$");
  std::smatch match;

  if (std::regex_search(version_string, match, version_regex)) {
    major = std::stoi(match[1]);
    minor = std::stoi(match[2]);
  } else {
    throw std::runtime_error("unable to construct version string from " + version_string);
  }
}

bool operator==(const ApiVersion& lhs, const ApiVersion& rhs) {
  return std::tie(lhs.major, lhs.minor) == std::tie(rhs.major, rhs.minor);
}

bool operator!=(const ApiVersion& lhs, const ApiVersion& rhs) {
  return std::tie(lhs.major, lhs.minor) != std::tie(rhs.major, rhs.minor);
}

bool operator<(const ApiVersion& lhs, const ApiVersion& rhs) {
  return std::tie(lhs.major, lhs.minor) < std::tie(rhs.major, rhs.minor);
}

bool operator<=(const ApiVersion& lhs, const ApiVersion& rhs) {
  return std::tie(lhs.major, lhs.minor) <= std::tie(rhs.major, rhs.minor);
}

bool operator>(const ApiVersion& lhs, const ApiVersion& rhs) {
  return std::tie(lhs.major, lhs.minor) > std::tie(rhs.major, rhs.minor);
}

bool operator>=(const ApiVersion& lhs, const ApiVersion& rhs) {
  return std::tie(lhs.major, lhs.minor) >= std::tie(rhs.major, rhs.minor);
}

// Output stream operator overload
std::ostream& operator<<(std::ostream& os, const ApiVersion& ver) {
  os << ver.major << "." << ver.minor;

  return os;
}
