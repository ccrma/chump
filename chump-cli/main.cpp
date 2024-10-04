// ChuMP (ChucK Manager of Programs) CLI main file.

//-----------------------------------------------------------------------------
// name: main()
// desc: command line chuck entry point
//-----------------------------------------------------------------------------

#include "chuck_version.h"
#include "exec.h"
#include "manager.h"
#include "util.h"

#include <filesystem>

namespace fs = std::filesystem;

int main( int argc, const char ** argv ) {
  if (argc < 2) {
    std::cerr << "incorrect number of argument (should be more than two)" << std::endl;
    return -1;
  }
  auto [cmd, args] = parseArgs(argc, argv);

  fs::path path = chumpDir();

  fs::create_directories(path);

  // Build manager and run command
  try {
    Manager* m = new Manager(chumpDir() / "packages.json", path, ChuckVersion::makeSystemVersion(), ApiVersion::makeSystemVersion(), whichOS(), true);
    execCommand(cmd, args, m);
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  return 0;
}
