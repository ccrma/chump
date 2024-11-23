#include <iostream>
#include <iomanip>
#include <tuple>
#include <vector>

#include "manager.h"

std::string printLogo();
void printPackage(Package pkg);
void printPackages(Manager* mgr);

void printPackage(Package pkg) {
  std::cout << pkg << std::endl;
}

// https://stackoverflow.com/questions/19580877/how-to-truncate-a-string-formating-c
std::string truncate(std::string str, size_t width, bool show_ellipsis=true) {
    if (str.length() > width) {
        if (show_ellipsis) {
            str.resize(width);
            return str + "...";
        }
        else {
            str.resize(width);
            return str;
        }
    }
    return str;
}

void printPackages(Manager* mgr, bool print_installed) {
  vector<Package> packages = mgr->listPackages();

  std::cout << std::left << std::setw(20) << "Name" << "| "
            << std::setw(12) << "Latest Ver." << "| "
            << std::setw(12) << "Installed?" << "| "
            << std::setw(12) << "Description" << std::endl;

  std::cout << std::setfill('-'); // in-between line from header to table

  std::cout << std::right << std::setw(21) << "|"
            << std::setw(14) << "|"
            << std::setw(14) << "|"
            << std::setw(14) << "" << std::endl;

  std::cout << std::setfill(' ');

  for (Package p: packages) {
    if (print_installed && !mgr->is_installed(p)) continue;

    optional<PackageVersion> latest = mgr->latestPackageVersion(p.name);

    string latest_version = "N/A";
    if (latest)
      latest_version = latest.value().getVersionString();

    string installed = mgr->is_installed(p) ? "yes" : "no";

    std::cout << std::left << std::setw(20) << truncate(p.name, 17, true) << "| "
              << std::setw(12) << latest_version << "| "
              << std::setw(12) << installed << "| "
              << std::setw(12) << truncate(p.description, 100, true)
              << std::endl;
  }
}


std::string printLogo() {
std::string logo = R"(
                                                 ss
                                                 rox        zop
                                                 xor        ony
              tkjiimtz             yrljjjjjjox    oo       qnx
                  zsuckerlrwzzxsliiimty           sov     wor
                        xrmjjjjmsz                 oo     poz
                                                   row   yoq
                               xqnrutchumpmv        oo   tov
                          zskiijnpmklry             uot  oo
                       yohhmu                        pnyzop
             uljjjjjjhippy                           znopoy
                                                      vnfo
                                                       vmr         zxwvx
                                                              zxxwwwy  ywwz
            oo  yy                                          wwwxxz       zxvx
        ysoooo  xw                                         xx               ww
      womsz     ywy                                        zwx              xx
     tmt         wx                                         ww              ww
    xnq          ww                   zz                    zwx             ww
   zno           ww                    yz     zz zyz         ww            xvz
   poy           ww                    zyz    xvuvvv   vx    ywxyxyzywxxwwxz
   oo            xw     zxy    yxz      zyz   xtv  wwwwzwvz  zuuxyxxy
   oo            zwx    wtw    zwx       yz   ywx  xtvy  wwz  xx
   tmntoooooopv   ww   wwww     yxz      zzz  zvw  zww   ww   xx
    tmoooxxywxy   ywz xwzxwz    zxyz     zzz  zvw       zwx   ww
                  zwx wx  ww    zz yyz   yy    ww       ywy   xw
                   xwvw    wvy  yy  zyyzyz     xx       xwz   zwy
                    vtx                z               zyz    xx
                                                               xw
                                                                xy
                --- ChuMP! the CHUck Manager of Packages ---
  )";

    return logo;
}
