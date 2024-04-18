// ChuMP (ChucK Manager of Programs) CLI main file.

//-----------------------------------------------------------------------------
// name: main()
// desc: command line chuck entry point
//-----------------------------------------------------------------------------

#include "exec.h"
#include "manager.h"
#include <filesystem>

namespace fs = std::filesystem;

int main( int argc, const char ** argv ) {
  if (argc < 2) {
    std::cerr << "incorrect number of argument (should be more than two)" << std::endl;
    return -1;
  }
  auto [cmd, args] = parseArgs(argc, argv);


  // create the necessary folders
  // TODO: os-specific

  // get home environment variable
  const char* env_home = std::getenv("HOME");
  fs::path path = fs::path(env_home);

  path = path / ".chuck" / "lib" / ".chump";
  fs::create_directories(path);

  // Build manager and run command
  Manager* m = new Manager("../data/packages.json");
  execCommand(cmd, args, m);

  return 0;
}
