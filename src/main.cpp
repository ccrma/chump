// ChuMP (ChucK Manager of Programs) CLI main file.

//-----------------------------------------------------------------------------
// name: main()
// desc: command line chuck entry point
//-----------------------------------------------------------------------------

#include "exec.h"

int main( int argc, const char ** argv ) {
  if (argc < 2) {
    std::cerr << "incorrect number of argument (should be more than two)" << std::endl;
    return -1;
  }

  auto [cmd, args] = parseArgs(argc, argv);
  execCommand(cmd, args);

  return 0;
}
