#include <iostream>
#include <tuple>
#include <vector>


// Parse arguments into a (command, arguments[]) tuple
std::tuple<std::string, std::vector<std::string>> parseArgs( int argc, const char ** argv ) {
  std::vector<std::string> arguments(argv + 1, argv + argc);

  std::string cmd = arguments[0];
  std::vector<std::string> args(arguments.begin()+1, arguments.end());

  return {cmd, args};
}

void execCommand(std::string cmd, std::vector<std::string> args) {
  if (cmd == "info") {
    // do stuff...
    std::cout << cmd << " ";
    for(auto arg: args) {
      std::cout << arg << " ";
    }
    std::cout << std::endl;
  } else {
    std::cerr << "command " << cmd << " is not a valid command." << std::endl;
  }
}
