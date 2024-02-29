#include <filesystem>
#include <iostream>

#include "fetch.h"

/*
  valid use cases:
  - chump info path/to/spec.json # directly link to the pkg file
  - chump info path/to/ # dir containing spec file
  - chump info www.package.com/ABSaturator.zip # the path is a url to be downloaded
  - chump info pkg-name # search pre-defined directories for the pkg name (or online pkg list)
 */
optional<Package> Fetch::fetch(std::string url) {
  if (!isURL(url)) {
    std::cerr << "Not a URL!" << std::endl;
    return {};
  }
  
  CURL *curl;
  FILE *fp;
  CURLcode res;

  // Create a temporary directory
  fs::path tempDir = fs::temp_directory_path();

  // Generate a unique temporary file name
  fs::path tempFilePath = tempDir / "downloaded_file.tmp";

  fp = fopen(tempFilePath.c_str(), "wb");
  if (!fp) {
    std::cerr << "Error opening file for writing" << std::endl;
    return {};
  }

  // Initialize libcurl
  curl = curl_easy_init();
  if (curl) {
    // Set URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // Set callback function to write data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

    // Set file to write to
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

    // Perform the request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      std::cerr << "Failed to download: " << curl_easy_strerror(res) << std::endl;
    }

    // Clean up
    curl_easy_cleanup(curl);
    fclose(fp);

    std::cout << "File downloaded successfully" << std::endl;
  } else {
    std::cerr << "Failed to initialize libcurl" << std::endl;
    return {};
  }

  return {};

}

// Callback function to write data into a file
size_t Fetch::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

// bool Fetch::isJSONFile(std::string path) {
//   // Check if the path has a json extension
//   return (path.size() > 5) && (path.substr(path.size() - 5) == ".json");
// }

// bool Fetch::isDirectory(std::string path) {
//   // Check if the path is a directory
//   return std::filesystem::is_directory(path);
// }

bool Fetch::isURL(std::string path) {
  // https://www.geeksforgeeks.org/check-if-an-url-is-valid-or-not-using-regular-expression/#
  const std::regex pattern("((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)");

  return regex_match(path, pattern);
}

bool Fetch::isPackageName(std::string path) {
  return path != "" && std::all_of(path.begin(), path.end(), ::isalnum);
}
