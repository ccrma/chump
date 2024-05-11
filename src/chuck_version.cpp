
#include "chuck_version.h"

#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <regex>


ChuckVersion::ChuckVersion() {
  // Query chuck to get verison string
  FILE* pipe = popen("chuck --query:version 2>&1", "r");
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }

  std::string version;

  constexpr int buffer_size = 128;
  char buffer[buffer_size];
  while (fgets(buffer, buffer_size, pipe) != nullptr) {
    version += buffer;
  }

  auto status = pclose(pipe);
  if (status == -1) {
    throw std::runtime_error("pclose() failed!");
  }

  set_version(version);
}

ChuckVersion::ChuckVersion(string version_string) {
  set_version(version_string);
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



ApiVersion::ApiVersion() {
  // Query chuck to get api version string
  FILE* pipe = popen("chuck --query:version_chugin_api 2>&1", "r");
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }

  std::string version;

  constexpr int buffer_size = 128;
  char buffer[buffer_size];
  while (fgets(buffer, buffer_size, pipe) != nullptr) {
    version += buffer;
  }

  auto status = pclose(pipe);
  if (status == -1) {
    throw std::runtime_error("pclose() failed!");
  }

  set_version(version);
}

ApiVersion::ApiVersion(string version_string) {
  set_version(version_string);
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
