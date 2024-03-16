// ChuMP (ChucK Manager of Programs) CLI main file.

//-----------------------------------------------------------------------------
// name: main()
// desc: command line chuck entry point
//-----------------------------------------------------------------------------

#include "exec.h"
#include "manager.h"

int main( int argc, const char ** argv ) {
  if (argc < 2) {
    std::cerr << "incorrect number of argument (should be more than two)" << std::endl;
    return -1;
  }
  auto [cmd, args] = parseArgs(argc, argv);

  Manager* m = new Manager("../data/packages.json");
  execCommand(cmd, args, m);

  return 0;
}
