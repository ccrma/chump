#include <filesystem>
#include <iostream>
#include <chrono>

#include "curses.h"

#include "fetch.h"
#include "util.h"

#include <openssl/sha.h>


Fetch::Fetch() {
  render = false;
}

Fetch::Fetch(bool render_tui) {
  render = render_tui;
}

// Callback function to update progress
int progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    // Calculate progress percentage
    double progress = (dlnow > 0) ? ((double)dlnow / (double)dltotal) : 0.0;

    int bar_width = 50; // Width of the progress bar
    int filled_width = progress * bar_width;

    // Get the current time point
    auto currentTime = std::chrono::system_clock::now();

    // Get the duration since the epoch
    auto durationSinceEpoch = currentTime.time_since_epoch();

    // Convert the duration to milliseconds
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(durationSinceEpoch);

    // Extract the milliseconds component
    long long milliseconds_count = milliseconds.count() % 1000;

    // Clear the screen
    erase();

    // Draw the progress bar
    // mvprintw(0, 0, "DownChucKing: [");
    mvprintw(0, 0, "DownChucKing ");

    if (milliseconds_count < 250) {
      printw("=>");
    } else if (milliseconds_count < 500) {
      printw("=v");
    } else if (milliseconds_count < 750) {
      printw("=<");
    } else {
      printw("=^");
    }

    printw(" [");

    for (int i = 0; i < bar_width; i++) {
        if (i < filled_width)
          printw("=");
        else if (i == filled_width)
          printw("v");
        else
          printw(" ");
    }
    printw("] %3d%% ", (int)(progress * 100));
    refresh();

    return 0;
}

//*******************************************
// Download file to proper package directory.
// Return true on success, False on failure.
//*******************************************
bool Fetch::fetch(std::string url, std::string dir, Package package, fs::path temp_dir) {
  if (!isURL(url)) {
    std::cerr << "Not a URL!" << std::endl;
    return false;
  }

  // If the file is in a directory, create it
  fs::create_directory(temp_dir / dir);

  std::string package_name = package.name;

  // struct progress data;
  CURL *curl;
  FILE *fp;
  CURLcode res;

  fs::path filename = fs::path(url).filename();

  // Generate a unique temporary file name
  fs::path tempFilePath = temp_dir / dir / filename;

#ifdef _MSC_VER
  fp = _wfopen(tempFilePath.c_str(), L"wb");
#else
  fp = fopen(tempFilePath.string().c_str(), "wb");
#endif

  if (!fp) {
    std::cerr << "Error opening file for writing" << std::endl;
    return false;
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

    // Set the progress callback function
    if (render)
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    // We don't want to write the error to a file if the request fails
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

    // Perform the request
    if (render) initscr();
    res = curl_easy_perform(curl);
    if (render) endwin();

    // Clean up
    curl_easy_cleanup(curl);
    fclose(fp);

    if (res != CURLE_OK) {
      std::cerr << "Failed to download: " << curl_easy_strerror(res) << std::endl;
      return false;
    }

  } else {
    std::cerr << "Failed to initialize libcurl" << std::endl;
    return false;
  }

  std::cout << "Successfully downloaded " << filename << "!" << std::endl;
  // std::cout << "hash: " << hash_file(tempFilePath) << std::endl;

  return true;
}

// Callback function to write data into a file
size_t Fetch::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

bool Fetch::isURL(std::string path) {
  // https://www.geeksforgeeks.org/check-if-an-url-is-valid-or-not-using-regular-expression/#
  const std::regex pattern("((http|https)://)(www.)?[a-zA-Z0-9@:%._\\+~#?&//=]{2,256}\\.[a-z]{2,6}\\b([-a-zA-Z0-9@:%._\\+~#?&//=]*)");

  return regex_match(path, pattern);
}

bool Fetch::fetch_manifest(std::string url, fs::path dir) {
  if (!isURL(url)) {
    std::cerr << "Not a URL!" << std::endl;
    return false;
  }

    // struct progress data;
  CURL *curl;
  FILE *fp;
  CURLcode res;

  fs::path tempFilePath = dir / "manifest.json";

  #ifdef _MSC_VER
  fp = _wfopen(tempFilePath.c_str(), L"wb");
#else
  fp = fopen(tempFilePath.string().c_str(), "wb");
#endif

  if (!fp) {
    std::cerr << "Error opening file for writing" << std::endl;
    return false;
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

    // Set the progress callback function
    if (render)
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    // We don't want to write the error to a file if the request fails
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

    // Perform the request
    if (render) initscr();
    res = curl_easy_perform(curl);
    if (render) endwin();

    // Clean up
    curl_easy_cleanup(curl);
    fclose(fp);

    if (res != CURLE_OK) {
      std::cerr << "Failed to download: " << curl_easy_strerror(res) << std::endl;
      return false;
    }

  } else {
    std::cerr << "Failed to initialize libcurl" << std::endl;
    return false;
  }

  std::cout << "Successfully downloaded manifest.json!" << std::endl;
  // std::cout << "hash: " << hash_file(tempFilePath) << std::endl;

  return true;
}
