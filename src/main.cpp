// ChuMP (ChucK Manager of Programs) CLI main file.

//-----------------------------------------------------------------------------
// name: main()
// desc: command line chuck entry point
//-----------------------------------------------------------------------------

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

  fs::path path = get_home_directory();

  // different operating systems have different install directories
#ifdef _WIN32
  path = path / "Documents" / "chuck" / "chugins" / ".chump";
#else // unix-systems (I know this!)
  path = path / ".chuck" / "lib" / ".chump";
#endif

  fs::create_directories(path);

  // Build manager and run command
  Manager* m = new Manager("../data/packages.json");
  execCommand(cmd, args, m);

  return 0;
}
