#include <iostream>
#include <tuple>
#include <vector>

#include "manager.h"

std::string printLogo();
void printPackage(Package pkg);
void printPackages(Manager* mgr);

void printPackage(Package pkg) {
  std::cout << pkg << std::endl;
}

void printPackages(Manager* mgr) {
  vector<Package> packages = mgr->listPackages();

  std::cout << "Name | Latest Ver. | Description" << std::endl;

  for (Package p: packages) {
    optional<PackageVersion> latest = mgr->latestPackageVersion(p.name);

    string latest_version = "N/A";
    if (latest)
      latest_version = latest.value().getVersionString();

    std::cout
      << p.name << " | "
      << latest_version << " | "
      << p.description << std::endl;
  }
}


std::string printLogo() {
  std::string logo = R"(
                                                            ss
                                                            rox        zop
                                                            xor        ony
                         tkjiimtz             yrljjjjjjox    oo       qnx
                             zsliiiilrwzzxsliiimty           sov     wor
                                   xrmjjjjmsz                 oo     poz
                                                              row   yoq
                                          xqnrutlhiihmv        oo   tov
                                     zskiijnpmklry             uot  oo
                                  yohhmu                        pnyzop
                        uljjjjjjjihpy                           znopoy
                                                                 vnfo
                                                                  vmr

                                                                                zxwvx
                                                                           zxxwwwy  ywwz
                 xx   yy                                              ywwwxxz         zxvx
           ysoooopuz  xw                                              xx                 ww
         womsz        ywy                                             zwx                xx
        tmt            wx                                              ww                ww
       xnq             ww                    zz                        zwx               ww
      zno              ww                     yz     zz zyz             ww              xvz
      poy              ww                     zyz    xvuvvv   vx        ywxyxyzywwxxxxwwxz
      oo               xw     zxy     yxz      zyz   xtv  wwwwzwvz      zuuxyxxy
      oo               zwx    wtw     zwx       yz   ywx  xtvy  wwz      xx
      tmnooooooopv      ww   wwww      yxz      zzz  zvw  zww   ww       xx
       tmoooxxywxy      ywz xwzxwz     zxyz     zzz  zvw       zwx       ww
                        zwx wx  ww     zz yyz   yy    ww       ywy       xw
                         xwvw    wvy   yy  zyyzyz     xx       xwz       zwy
                          vtx                 z                zyz        xx
                                                                          xw
                                                                           xy
                           the CHUck Manager of Packages (chump)
)";

    return logo;
}
