#include <iostream>
#include <tuple>
#include <vector>

#include "manager.h"

void printLogo();

// Parse arguments into a (command, arguments[]) tuple
std::tuple<std::string, std::vector<std::string>> parseArgs( int argc, const char ** argv ) {
  std::vector<std::string> arguments(argv + 1, argv + argc);

  std::string cmd = arguments[0];
  std::vector<std::string> args(arguments.begin()+1, arguments.end());

  return {cmd, args};
}

void execCommand(std::string cmd, std::vector<std::string> args, Manager* manager) {
  if (cmd == "info") {
    // if (args.size() != 1) {
    //   std::cerr << "\"chump info\" should have one argument" << std::endl;
    //   return;
    // }

    // auto p = manager->fetch->fetch(args[0]);

    // if (p) {
    //   std::cout << p.value() << std::endl;
    // } else {
    //   std::cout << "package [" << args[0] << "] not found" << std::endl;
    // }

  } else if (cmd == "install") {
    // manager->fetch->fetch("https://ccrma.stanford.edu/~nshaheed/220a/hw4/hw4.wav");
    // manager->fetch->fetch("https://ccrma.stanford.edu/~nshaheed/rave_models/chafe_cello.ts");
    manager->install(args[0]);
  } else if (cmd == "help") {
    printLogo();
  } else {
    std::cerr << "command " << cmd << " is not a valid command." << std::endl;
  }
}


void printLogo() {
  std::cout << "                                                                                                                       AAA                                                                   " << std::endl;
std::cout << "                                                                                                                       AAA                                                                   " << std::endl;
std::cout << "                                                                                                                       A A                    AAA                                            " << std::endl;
std::cout << "                                                                                                                       AAAA                  AAAA                                            " << std::endl;
std::cout << "                                                                                                                       AAAA                 AAAAA                                            " << std::endl;
std::cout << "                                                                                                                       AAAA                 AAAAA                                            " << std::endl;
std::cout << "                                                 AAAAAAAA                                               OAAAAAG         A A                AAAAA                                             " << std::endl;
std::cout << "                                                 AAK  AAAAAAAAA                             AAAAAAAAAAA Y     A         AAAA              AA AA                                              " << std::endl;
std::cout << "                                                 AAAAAAAAAA AAAAAAAA                    AAAAAAA     AAA  AAAAAG         AAAAA             AAAA                                               " << std::endl;
std::cout << "                                                        AAAAAAAAA AAAAAAAAA       AAAAAAAAAAAAAAAAAAAAA                  AAAA            AAAAA                                               " << std::endl;
std::cout << "                                                              AAAAAAAA  AAA       AAA AAAAAAA                            AAAA            AAAA                                                " << std::endl;
std::cout << "                                                                   AAAAAAAA       AAAAAAA                                 AAAA          AAAA                                                 " << std::endl;
std::cout << "                                                                                                                          AAAA         AAAAA                                                 " << std::endl;
std::cout << "                                                                                                                          AAAAA        AAAA                                                  " << std::endl;
std::cout << "                                                                                                 AAAAAAAAAAA               AAAAA       AAAA                                                  " << std::endl;
std::cout << "                                                                                   AAAAAAAAA AAAAAA   AAAAAA                AAAA       AAAA                                                  " << std::endl;
std::cout << "                                                                               AAAAAAA    AA AA AAAAAAAAAAAA                AAAAA     AAAAA                                                  " << std::endl;
std::cout << "                                                                          AAAAAAA AAAAAAAAAA AAAAA                           AAAA     AAAA                                                   " << std::endl;
std::cout << "                                                                      AAAAAAAEAAAAAA                                         AAAAA    A AA                                                   " << std::endl;
std::cout << "                                                                    AAAAAAAAAAAA                                              AAAAA  AAAA                                                    " << std::endl;
std::cout << "                                               AAAAAAAAAAAAAAAA AAAAAAAAAAA                                                   UAAAA  AAAA                                                    " << std::endl;
std::cout << "                                               AAA         OAAA AAAAAAAA                                                       AAAAAAA AA                                                    " << std::endl;
std::cout << "                                               AAAAAAAAAAAAAAAA AAAAAA                                                          AAAAAAAA                                                     " << std::endl;
std::cout << "                                                                                                                                 AAAAAAA                                                     " << std::endl;
std::cout << "                                                                                                                                 AAAKAAA                                                     " << std::endl;
std::cout << "                                                                                                                                  AAAAAA                                                     " << std::endl;
std::cout << "                                                                                                                                   AAAAA                                                     " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                                             " << std::endl;
std::cout << "                                                                                                                                                                  AAAA                       " << std::endl;
std::cout << "                                                                                                                                                             AAAAA A AAAA                    " << std::endl;
std::cout << "                                                                                                                                                         AAAA      AA   AAA                  " << std::endl;
std::cout << "                                                                                                                                                   AAAAA A  FAAAAAC  AAA   AAA               " << std::endl;
std::cout << "                                           AAAW                                                                                           AAAAAAA  A   H AAAAA         PAAA A AAA            " << std::endl;
std::cout << "                                AAAAAA     AAAA                                                                                           AA   AA  AAAAAP                 AAA A AA           " << std::endl;
std::cout << "                         AAAAAAAAA  AA     CA A                                                                                           AAAAAAA                            AAA AA          " << std::endl;
std::cout << "                      AAAAA   AAAAAAAA      AAAA                                                                                          AA A                                 AA A          " << std::endl;
std::cout << "                   AAAAAAAAAAAAA            AA A                                                                                           A AA                                 A A          " << std::endl;
std::cout << "                  AAAAAAAA                   A A                                                                                           AA A                                 A A          " << std::endl;
std::cout << "                 AA AAAA                     A A                                                                                            A AA                                A A          " << std::endl;
std::cout << "                AAAAAA                       A A                                                                                            AA A                                A A          " << std::endl;
std::cout << "               AAAAA                         A AH                                        AAF                                                 A AA                               A A          " << std::endl;
std::cout << "              AAAAAA                         AAAA                                        A AM                                                AAAA                              AA A          " << std::endl;
std::cout << "              AAAAA                           A A                                        GA AV                                                A A                             AA AA          " << std::endl;
std::cout << "             AAAAA                            A A                                         LA A            AAM AAAAAA                          AAAA                          DAA AA           " << std::endl;
std::cout << "             AAAA                             A A                                          A AA          AAAAAA    AA  AAAAAAP                 A A            AAAAAAAAAAAAAAVA AA            " << std::endl;
std::cout << "             A AF                             A A                           AA              A AS         AA A  AAAA AG AA   AA                AAAAAAAAAAAAAAANAA          AA AAA             " << std::endl;
std::cout << "             A A                              A A            AAA           AAAA             NA AT        AA AAAH  AAAA A AAAAAAA              AA     A A    AANAAAAAAAAAAAAA                 " << std::endl;
std::cout << "            AAAA                              AAAA          AAAA           AA AA             LA A        AA AA    ZA AAAAA CAA AAA            AAAAAAAAAAAAAAAA                               " << std::endl;
std::cout << "            AAAA                              AA A         AAAAA            A OAT             A AA       AA A      A AA A   AAAAAA              A A                                          " << std::endl;
std::cout << "            AAAA                               A A        AA AOAA           AJ QA              A A        A A      AAUSAA      A A              A A                                          " << std::endl;
std::cout << "            AAAAAAAAAAAAAAAAAX                 AAAA       A AAAAA            AA A              A A        A A      AAAAA      AA A              A A                                          " << std::endl;
std::cout << "             AAA           AAV                 AA A      AAAASA A             A A               A A       A A       AAAA      AAAA              A A                                          " << std::endl;
std::cout << "             AAAAAAAAAAAAAAAAX                  AAAA    AA AN A A            KA  MAV            A A       A A        AA       A AA              A A                                          " << std::endl;
std::cout << "                                                AAAA    A AA  AAAA           A YA  AA           A A       A A                AA A               A A                                          " << std::endl;
std::cout << "                                                 A A   AAAA   AA AA          A A AA ZAR        AA A       A A                AAAA               AAAA                                         " << std::endl;
std::cout << "                                                 AAAA AA AA    AA A          A A  OAK AA      AI AX       A A                AAAA               NA A                                         " << std::endl;
std::cout << "                                                 AA A AAAA      AAAAAD       A AU   AA QAA  BA  A         A A                AAAA                A A                                         " << std::endl;
std::cout << "                                                  AAAAA A       AAA AA       A A     GAQ A  A YA          AAA                A AU                AAAA                                        " << std::endl;
std::cout << "                                                  AA AAAA         AAAD       RAA       AAA  AAA           AAA                AAA                  A A                                        " << std::endl;
std::cout << "                                                   AAAAA                                                                     AAA                  A A                                        " << std::endl;
std::cout << "                                                   AAAAA                                                                                          A A                                        " << std::endl;
std::cout << "                                                                                                                                                  A AA                                       " << std::endl;
std::cout << "                                                                                                                                                  AA A                                       " << std::endl;
std::cout << "                                                                                                                                                   AAA                                       " << std::endl;
}
