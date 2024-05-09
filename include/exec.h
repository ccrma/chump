#include <iostream>
#include <tuple>
#include <vector>

#include "manager.h"

void printLogo();
void printPackage(Package pkg);
void printPackages(Manager* mgr);

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
      std::cerr << "\"chump info\" should follow the format \"chump info PACKAGENAME\"" << std::endl;
      return;
    }

    std::string pkg_name = args[0];
    optional<Package> pkg = manager->getPackage(pkg_name);

    if (!pkg) {
      std::cerr << "Unable to find package " << pkg_name << std::endl;
      return;
    }

    printPackage(pkg.value());

  } else if (cmd == "list") {
    if (args.size() != 0) {
      std::cerr << "\"chump list\" should have no arguments" << std::endl;
      return;
    }

    printPackages(manager);
  } else if (cmd == "install") {
    // manager->fetch->fetch("https://ccrma.stanford.edu/~nshaheed/220a/hw4/hw4.wav");
    // manager->fetch->fetch("https://ccrma.stanford.edu/~nshaheed/rave_models/chafe_cello.ts");
    manager->install(args[0]);
  } else if (cmd == "uninstall") {
    manager->uninstall(args[0]);
  } else if (cmd == "help") {
    printLogo();
  } else {
    std::cerr << "command " << cmd << " is not a valid command." << std::endl;
  }
}

void printPackage(Package pkg) {
  std::cout << pkg << std::endl;
}

void printPackages(Manager* mgr) {
  vector<Package> packages = mgr->listPackages();
  for (Package p: packages) {
    std::cout << p.name << std::endl;
  }
}


void printLogo() {
  std::cout << std::endl;
  std::cout << "                                                            ss                                  " << std::endl;
  std::cout << "                                                            rox        zop                      " << std::endl;
  std::cout << "                                                            xor        ony                      " << std::endl;
  std::cout << "                         tkjiimtz             yrljjjjjjox    oo       qnx                       " << std::endl;
  std::cout << "                             zsliiiilrwzzxsliiimty           sov     wor                        " << std::endl;
  std::cout << "                                   xrmjjjjmsz                 oo     poz                        " << std::endl;
  std::cout << "                                                              row   yoq                         " << std::endl;
  std::cout << "                                          xqnrutlhiihmv        oo   tov                         " << std::endl;
  std::cout << "                                     zskiijnpmklry             uot  oo                          " << std::endl;
  std::cout << "                                  yohhmu                        pnyzop                          " << std::endl;
  std::cout << "                        uljjjjjjjihpy                           znopoy                          " << std::endl;
  std::cout << "                                                                 vnfo                           " << std::endl;
  std::cout << "                                                                  vmr                           " << std::endl;
  std::cout << "                                                                                                " << std::endl;
  std::cout << "                                                                                                " << std::endl;
  std::cout << "                                                                                                " << std::endl;
  std::cout << "                                                                                                " << std::endl;
  std::cout << "                                                                                                " << std::endl;
  std::cout << "                                                                                                " << std::endl;
  std::cout << "                                                                                zxwvx           " << std::endl;
  std::cout << "                                                                           zxxwwwy  ywwz        " << std::endl;
  std::cout << "                 xx   yy                                              ywwwxxz         zxvx      " << std::endl;
  std::cout << "           ysoooopuz  xw                                              xx                 ww     " << std::endl;
  std::cout << "         womsz        ywy                                             zwx                xx     " << std::endl;
  std::cout << "        tmt            wx                                              ww                ww     " << std::endl;
  std::cout << "       xnq             ww                    zz                        zwx               ww     " << std::endl;
  std::cout << "      zno              ww                     yz     zz zyz             ww              xvz     " << std::endl;
  std::cout << "      poy              ww                     zyz    xvuvvv   vx        ywxyxyzywwxxxxwwxz      " << std::endl;
  std::cout << "      oo               xw     zxy     yxz      zyz   xtv  wwwwzwvz      zuuxyxxy                " << std::endl;
  std::cout << "      oo               zwx    wtw     zwx       yz   ywx  xtvy  wwz      xx                     " << std::endl;
  std::cout << "      tmnooooooopv      ww   wwww      yxz      zzz  zvw  zww   ww       xx                     " << std::endl;
  std::cout << "       tmoooxxywxy      ywz xwzxwz     zxyz     zzz  zvw       zwx       ww                     " << std::endl;
  std::cout << "                        zwx wx  ww     zz yyz   yy    ww       ywy       xw                     " << std::endl;
  std::cout << "                         xwvw    wvy   yy  zyyzyz     xx       xwz       zwy                    " << std::endl;
  std::cout << "                          vtx                 z                zyz        xx                    " << std::endl;
  std::cout << "                                                                          xw                    " << std::endl;
  std::cout << "                                                                           xy                   " << std::endl;
  std::cout << "                                                                                                " << std::endl;
}
