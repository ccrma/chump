#include <filesystem>
#include <iostream>
#include <chrono>
#include <sstream>

#include "curses.h"

#include "fetch.h"
#include "util.h"

#include <openssl/sha.h>

#include <unistd.h>


Fetch::Fetch() {
  render = false;
}

Fetch::Fetch(bool render_tui) {
  render = render_tui;
}

// struct to pass progress data to libcurl progress callback
struct curl_progress {
  string packageName;
  string fileName;
};

// Callback function to update progress
int progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow) {
    if (!clientp) {
      std::cerr << "progress callback recieved empty clientp" << std::endl;
      return -1;
    }
    // metadata for the progress bar
    struct curl_progress *memory = static_cast<struct curl_progress*>(clientp);

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
    // erase();

    // Draw the progress bar
    // mvprintw(0, 0, "DownChucKing: [");
    // mvprintw(0, 0, "DownChucKing Package %s (%s)\n", memory->packageName.c_str(), memory->fileName.c_str());

    string line;

    if (milliseconds_count < 250) {
      line = "=>";
    } else if (milliseconds_count < 500) {
      line = "=v";
    } else if (milliseconds_count < 750) {
      line = "=<";
    } else {
      line = "=^";
    }

    line += " [";
    for (int i = 0; i < bar_width; i++) {
        if (i < filled_width)
          line += "=";
        else if (i == filled_width)
          line += "v";
        else
          line += " ";
    }
    line += string("] ") + std::to_string( (int)(progress * 100 + .5) ) + "%";
    // print it
    // fprintf( stderr, "\r%s", line.c_str() );
    std::cerr << "\r" << line.c_str();

    // mvprintw(1, 0, memory->fileName.c_str());
    // refresh();

    return 0;
}

//*******************************************
// Download file to proper package directory.
// Return true on success, False on failure.
//*******************************************
bool Fetch::fetch(std::string url, fs::path dir,
                  Package package, fs::path temp_dir,
                  FileType file_type, string checksum) {
  if (!isURL(url)) {
    std::cerr << "Not a URL!" << std::endl;
    return false;
  }

  fs::path ft_dir = fileTypeToDir(file_type);

  // If the file is in a directory, create it
  fs::create_directory(temp_dir / ft_dir );
  fs::create_directory(temp_dir / ft_dir / dir);

  std::string package_name = package.name;

  // struct progress data;
  CURL *curl;
  FILE *fp;
  CURLcode res;

  fs::path filename = fs::path(url).filename();

  // Generate a unique temporary file name
  fs::path tempFilePath = temp_dir / dir / ft_dir / filename;

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

    struct curl_progress data;
    data.packageName = package.name;
    data.fileName = filename.string();

    // Set the progress callback function
    if (render) {
      curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &data);
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
    }
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    // We don't want to write the error to a file if the request fails
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

    // Perform the request
    //if (render) initscr();
    string line = "down-chucking package: '";
    line += package.name + "' file: '" + filename.string() + "'\n";
    // print it
    // fprintf( stderr, "%s", line.c_str() );
    std::cerr << line;

    res = curl_easy_perform(curl);
    //if (render) endwin();

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

  if (hash_file(tempFilePath) != checksum) {
    std::cerr << "the downloaded file (" << url
              << ") does not match expected hash - aborting" << std::endl;

    return false;
  }

    std::cerr << "\r |- successfully downloaded " << filename << "!                                                             " << std::endl;;

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

    struct curl_progress data;
    data.packageName = "manifest";
    data.fileName = "manifest.json";

    // Set the progress callback function
    if (render) {
      curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &data);
      // curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
      curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
    }
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    // We don't want to write the error to a file if the request fails
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, true);

    // Perform the request
    if (render) initscr();
    res = curl_easy_perform(curl);
      usleep( 10000 );
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

  std::cerr << "Successfully downloaded manifest.json!" << std::endl;
  // std::cerr << "hash: " << hash_file(tempFilePath) << std::endl;

  return true;
}
