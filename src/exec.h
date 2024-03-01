#include <iostream>
#include <tuple>
#include <vector>

#include "manager.h"


// Parse arguments into a (command, arguments[]) tuple
std::tuple<std::string, std::vector<std::string>> parseArgs( int argc, const char ** argv ) {
  std::vector<std::string> arguments(argv + 1, argv + argc);

  std::string cmd = arguments[0];
  std::vector<std::string> args(arguments.begin()+1, arguments.end());

  return {cmd, args};
}

void execCommand(std::string cmd, std::vector<std::string> args, Manager* manager) {
  if (cmd == "info") {
    if (args.size() != 1) {
      std::cerr << "\"chump info\" should have one argument" << std::endl;
      return;
    }

    auto p = manager->fetch->fetch(args[0]);

    if (p) {
      std::cout << p.value() << std::endl;
    } else {
      std::cout << "package [" << args[0] << "] not found" << std::endl;
    }

  } else if (cmd == "install") {
    // manager->fetch->fetch("https://ccrma.stanford.edu/~nshaheed/220a/hw4/hw4.wav");
    manager->fetch->fetch("https://ccrma.stanford.edu/~nshaheed/rave_models/chafe_cello.ts");
  } else {
    std::cerr << "command " << cmd << " is not a valid command." << std::endl;
  }
}
