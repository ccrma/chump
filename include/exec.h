#include <iostream>
#include <sstream>
#include <iomanip>
#include <tuple>
#include <vector>
#include <random>
#include <cctype>
#include <thread>
#include <chrono>
#include <stdlib.h>

#include "manager.h"
#include "util.h"

#if defined(_WIN32)
#include <Windows.h>
#else
#include <sys/ioctl.h>
#endif

using std::endl;

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

void get_terminal_size(int& width, int& height) {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = (int)(csbi.srWindow.Right-csbi.srWindow.Left+1);
    height = (int)(csbi.srWindow.Bottom-csbi.srWindow.Top+1);
#elif defined(__linux__)
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    width = (int)(w.ws_col);
    height = (int)(w.ws_row);
#endif // Windows/Linux
}

std::string jumbleColors(std::string str, int counter) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0,3);

  string output = "";

  for(std::string::size_type i = 0; i < str.size(); ++i) {
    int val = distribution(generator);
    val = (val + counter) % 4;
    // int val = rand() % 4;

    int bold = distribution(generator) % 2;

    std::string substr = str.substr(i, 1);

    int bg = distribution(generator);
    bg = (bg + counter) % 4;
    bg = (counter + i) % 4 ;

    if (std::isspace(str[i])) {
      output += substr;
      continue;
    }

    switch(val) {
    case 0:
      output += TC::green(substr, bold);
      break;
    case 1:
      output += TC::orange(substr, TRUE);
      break;
    case 2:
      output += TC::blue(substr, bold);
      break;
    case 3:
      output += TC::magenta(substr, bold);
      break;
    default:
      break;
    }
  }

  return output;
}

std::string riverColors(std::string str, int counter) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0,3);

  string output = "";

  for(std::string::size_type i = 0; i < str.size(); ++i) {
    int val = distribution(generator);
    val = (val + counter) % 4;
    // int val = rand() % 4;

    int bold = distribution(generator) % 2;

    std::string substr = str.substr(i, 1);

    int bg = distribution(generator);
    bg = (bg + counter) % 4;
    bg = (counter + i) % 4 ;

    if (std::isspace(str[i])) {
      switch(bg) {
      case 0:
        output += TC::set(TC::BG_CYAN);
        break;
      case 1:
        output += TC::set(TC::BG_MAGENTA);
        break;
      case 2:
        output += TC::set(TC::BG_WHITE);
        break;
      case 3:
        output += TC::set(TC::BG_DEFAULT);
        break;
      default:
        break;
      }
      output += substr + TC::reset();
      continue;
    }

    output += TC::set(TC::DIM);
    switch(val) {
    case 0:
      output += substr; // TC::default(substr, bold);
      break;
    case 1:
      output += TC::red(substr, TRUE);
      break;
    case 2:
      output += TC::color(TC::FG_BLACK, substr, bold);
      break;
    case 3:
      output += TC::magenta(substr, bold);
      break;
    default:
      break;
    }
  }

  return output;
}

std::string dimColors(std::string str, int counter) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0,3);

  string output = "";

  for(std::string::size_type i = 0; i < str.size(); ++i) {
    int val = distribution(generator);
    val = (val + counter) % 4;

    int bold = distribution(generator) % 2;

    std::string substr = str.substr(i, 1);

    int bg = distribution(generator);
    bg = (bg + counter) % 4;
    bg = (int(counter / 2) + i) % 4 ;

    output += TC::set(TC::DIM);

    if (std::isspace(str[i]) && substr != "\n") {
      switch(bg) {
      case 0:
        output += TC::red(".");
        break;
      case 1:
        output += TC::magenta(".");
        break;
      case 2:
        output += TC::color(TC::FG_BLACK, ".", bold);
        break;
      case 3:
        output += TC::color(TC::FG_WHITE, ".", bold);
        break;
      default:
        break;
      }
      output += TC::reset();
      continue;
    }

    if (std::isspace(str[i])) {
      output += substr;
      continue;
    }

    switch(val) {
    case 0:
      output += substr; // TC::default(substr, bold);
      break;
    case 1:
      output += TC::red(substr, FALSE);
      break;
    case 2:
      output += TC::color(TC::FG_BLACK, substr, bold);
      break;
    case 3:
      output += TC::magenta(substr, bold);
      break;
    default:
      break;
    }
  }

  return output;
}

std::string bedtimeColors(std::string str, int counter) {
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0,3);

  string output = "";

  for(std::string::size_type i = 0; i < str.size(); ++i) {
    int val = distribution(generator);
    val = (val + counter) % 4;

    int bold = distribution(generator) % 2;

    std::string substr = str.substr(i, 1);

    int bg = distribution(generator);
    bg = (bg + counter) % 4;
    bg = (counter + i) % 4 ;

    output += TC::set(TC::DIM);

    if (std::isspace(str[i])) {
      switch(bg) {
      case 0:
        output += TC::red(".");
        break;
      case 1:
        output += TC::magenta(".");
        break;
      case 2:
        output += TC::color(TC::FG_BLACK, ".", bold);
        break;
      case 3:
        output += TC::color(TC::FG_WHITE, ".", bold);
        break;
      default:
        break;
      }
      output += TC::reset();
      continue;
    }

    switch(val) {
    case 0:
      output += substr; // TC::default(substr, bold);
      break;
    case 1:
      output += TC::red(substr, FALSE);
      break;
    case 2:
      output += TC::color(TC::FG_BLACK, substr, bold);
      break;
    case 3:
      output += TC::magenta(substr, bold);
      break;
    default:
      break;
    }
  }

  return output;
}

std::string bedtimeLogo(int counter) {
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

  auto num_lines = std::count( logo.begin(), logo.end(), '\n' );
  num_lines += ( !logo.empty() && logo.back() != '\n' );

  logo = bedtimeColors(logo, counter);

  return logo;
}

std::string dimLogo(int counter) {
  int width=0, height=0;
  get_terminal_size(width, height);

  // 81 is the width of the chump logo
  int padding_len = (width - 81) / 2;
  if (padding_len < 0) padding_len = 0;

  std::string padding = "";

  for (int i = 0; i < padding_len; i++) {
    padding += " ";
  }

  std::vector<std::string> logo = {
    "                                                                                 ",
    "                                                 ss                              ",
    "                                                 rox        zop                  ",
    "                                                 xor        ony                  ",
    "              tkjiimtz             yrljjjjjjox    oo       qnx                   ",
    "                  zsuckerlrwzzxsliiimty           sov     wor                    ",
    "                        xrmjjjjmsz                 oo     poz                    ",
    "                                                   row   yoq                     ",
    "                               xqnrutchumpmv        oo   tov                     ",
    "                          zskiijnpmklry             uot  oo                      ",
    "                       yohhmu                        pnyzop                      ",
    "             uljjjjjjhippy                           znopoy                      ",
    "                                                      vnfo                       ",
    "                                                       vmr         zxwvx         ",
    "                                                              zxxwwwy  ywwz      ",
    "            oo  yy                                          wwwxxz       zxvx    ",
    "        ysoooo  xw                                         xx               ww   ",
    "      womsz     ywy                                        zwx              xx   ",
    "     tmt         wx                                         ww              ww   ",
    "    xnq          ww                   zz                    zwx             ww   ",
    "   zno           ww                    yz     zz zyz         ww            xvz   ",
    "   poy           ww                    zyz    xvuvvv   vx    ywxyxyzywxxwwxz     ",
    "   oo            xw     zxy    yxz      zyz   xtv  wwwwzwvz  zuuxyxxy            ",
    "   oo            zwx    wtw    zwx       yz   ywx  xtvy  wwz  xx                 ",
    "   tmntoooooopv   ww   wwww     yxz      zzz  zvw  zww   ww   xx                 ",
    "    tmoooxxywxy   ywz xwzxwz    zxyz     zzz  zvw       zwx   ww                 ",
    "                  zwx wx  ww    zz yyz   yy    ww       ywy   xw                 ",
    "                   xwvw    wvy  yy  zyyzyz     xx       xwz   zwy                ",
    "                    vtx                z               zyz    xx                 ",
    "                                                               xw                ",
    "                                                                xy               ",
    "                --- ChuMP! the CHUck Manager of Packages ---                     ",
    "                                                                                 ",
  };

    // auto num_lines = std::count( logo.begin(), logo.end(), '\n' );
    // num_lines += ( !logo.empty() && logo.back() != '\n' );

  std::string logo_combined;

  for (auto line: logo) {
    logo_combined += padding + line + padding + "\n";
  }

  logo_combined = dimColors(logo_combined, counter);

  return logo_combined;
}

std::string riverLogo(int counter) {
  int width=0, height=0;
  get_terminal_size(width, height);

  // 81 is the width of the chump logo
  int padding_len = (width - 81) / 2;
  if (padding_len < 0) padding_len = 0;

  std::string padding = "";

  for (int i = 0; i < padding_len; i++) {
    padding += " ";
  }

  std::vector<std::string> logo = {
    "                                                 ss            ",
    "                                                 rox        zop ",
    "                                                 xor        ony ",
    "              tkjiimtz             yrljjjjjjox    oo       qnx ",
    "                  zsuckerlrwzzxsliiimty           sov     wor ",
    "                        xrmjjjjmsz                 oo     poz ",
    "                                                   row   yoq ",
    "                               xqnrutchumpmv        oo   tov ",
    "                          zskiijnpmklry             uot  oo ",
    "                       yohhmu                        pnyzop ",
    "             uljjjjjjhippy                           znopoy ",
    "                                                      vnfo              ",
    "                                                       vmr         zxwvx ",
    "                                                              zxxwwwy  ywwz ",
    "            oo  yy                                          wwwxxz       zxvx ",
    "        ysoooo  xw                                         xx               ww ",
    "      womsz     ywy                                        zwx              xx ",
    "     tmt         wx                                         ww              ww ",
    "    xnq          ww                   zz                    zwx             ww ",
    "   zno           ww                    yz     zz zyz         ww            xvz ",
    "   poy           ww                    zyz    xvuvvv   vx    ywxyxyzywxxwwxz ",
    "   oo            xw     zxy    yxz      zyz   xtv  wwwwzwvz  zuuxyxxy       ",
    "   oo            zwx    wtw    zwx       yz   ywx  xtvy  wwz  xx ",
    "   tmntoooooopv   ww   wwww     yxz      zzz  zvw  zww   ww   xx ",
    "    tmoooxxywxy   ywz xwzxwz    zxyz     zzz  zvw       zwx   ww ",
    "                  zwx wx  ww    zz yyz   yy    ww       ywy   xw ",
    "                   xwvw    wvy  yy  zyyzyz     xx       xwz   zwy ",
    "                    vtx                z               zyz    xx ",
    "                                                               xw ",
    "                                                                xy ",
    "                --- ChuMP! the CHUck Manager of Packages ---      ",
  };

  std::string logo_combined;

  for (auto line: logo) {
    logo_combined += padding + line +"\n";
  }

  logo_combined = riverColors(logo_combined, counter);

  return logo_combined;
}

std::string jumbleLogo(int counter) {
  int width=0, height=0;
  get_terminal_size(width, height);

  // 81 is the width of the chump logo
  int padding_len = (width - 81) / 2;
  if (padding_len < 0) padding_len = 0;

  std::string padding = "";

  for (int i = 0; i < padding_len; i++) {
    padding += " ";
  }

  std::vector<std::string> logo = {
    "                                                 ss",
    "                                                 rox        zop",
    "                                                 xor        ony",
    "              tkjiimtz             yrljjjjjjox    oo       qnx",
    "                  zsuckerlrwzzxsliiimty           sov     wor",
    "                        xrmjjjjmsz                 oo     poz",
    "                                                   row   yoq",
    "                               xqnrutchumpmv        oo   tov",
    "                          zskiijnpmklry             uot  oo",
    "                       yohhmu                        pnyzop",
    "             uljjjjjjhippy                           znopoy",
    "                                                      vnfo",
    "                                                       vmr         zxwvx",
    "                                                              zxxwwwy  ywwz",
    "            oo  yy                                          wwwxxz       zxvx",
    "        ysoooo  xw                                         xx               ww",
    "      womsz     ywy                                        zwx              xx",
    "     tmt         wx                                         ww              ww",
    "    xnq          ww                   zz                    zwx             ww",
    "   zno           ww                    yz     zz zyz         ww            xvz",
    "   poy           ww                    zyz    xvuvvv   vx    ywxyxyzywxxwwxz",
    "   oo            xw     zxy    yxz      zyz   xtv  wwwwzwvz  zuuxyxxy",
    "   oo            zwx    wtw    zwx       yz   ywx  xtvy  wwz  xx",
    "   tmntoooooopv   ww   wwww     yxz      zzz  zvw  zww   ww   xx",
    "    tmoooxxywxy   ywz xwzxwz    zxyz     zzz  zvw       zwx   ww",
    "                  zwx wx  ww    zz yyz   yy    ww       ywy   xw",
    "                   xwvw    wvy  yy  zyyzyz     xx       xwz   zwy",
    "                    vtx                z               zyz    xx",
    "                                                               xw",
    "                                                                xy",
    "                --- ChuMP! the CHUck Manager of Packages ---",
  };

  std::string logo_combined;

  for (auto line: logo) {
    logo_combined += padding + line +"\n";
  }

  logo_combined = jumbleColors(logo_combined, counter);

  return logo_combined;
}

std::string printLogo() {
  std::ostringstream o;
  o << TC::green("                                                 ") << TC::orange("              ") << "               " << endl;
  o << TC::green("                                                 ") << TC::orange("ss            ") << "               " << endl;
  o << TC::green("                                                 ") << TC::orange("rox        zop") << "               " << endl;
  o << TC::green("                                                 ") << TC::orange("xor        ony") << "               " << endl;
  o << TC::green("              tkjiimtz             yrljjjjjjox   ") << TC::orange(" oo       qnx ") << "               " << endl;
  o << TC::green("                  zsuckerlrwzzxsliiimty          ") << TC::orange(" sov     wor  ") << "               " << endl;
  o << TC::green("                        xrmjjjjmsz               ") << TC::orange("  oo     poz  ") << "               " << endl;
  o << TC::green("                                                 ") << TC::orange("  row   yoq   ") << "               " << endl;
  o << TC::green("                               xqnrutchumpmv     ") << TC::orange("   oo   tov   ") << "               " << endl;
  o << TC::green("                          zskiijnpmklry          ") << TC::orange("   uot  oo    ") << "               " << endl;
  o << TC::green("                       yohhmu                    ") << TC::orange("    pnyzop    ") << "               " << endl;
  o << TC::green("             uljjjjjjhippy                       ") << TC::orange("    znopoy    ") << "               " << endl;
  o << TC::green("                                                 ") << TC::orange("     vnfo     ") << "               " << endl;
  o << TC::green("                                                 ") << TC::orange("      vmr     ") <<                                  TC::orange("    zxwvx      ") << endl;
  o << TC::green("                                                 ") << TC::orange("             ") <<                                  TC::orange("zxxwwwy  ywwz   ") << endl;
  o << TC::blue("            oo ") << TC::red(" yy            ") << TC::orange("              ") << TC::magenta("                ") << TC::orange("wwwxxz       zxvx ") << endl;
  o << TC::blue("        ysoooo ") << TC::red(" xw            ") << TC::orange("              ") << TC::magenta("              ") << TC::orange(" xx               ww") << endl;
  o << TC::blue("      womsz    ") << TC::red(" ywy           ") << TC::orange("              ") << TC::magenta("              ") << TC::orange(" zwx              xx") << endl;
  o << TC::blue("     tmt       ") << TC::red("  wx           ") << TC::orange("              ") << TC::magenta("               ") << TC::orange(" ww              ww") << endl;
  o << TC::blue("    xnq        ") << TC::red("  ww           ") << TC::orange("        zz    ") << TC::magenta("               ") << TC::orange(" zwx             ww") << endl;
  o << TC::blue("   zno         ") << TC::red("  ww           ") << TC::orange("         yz   ") << TC::magenta("  zz zyz        ") << TC::orange(" ww            xvz") << endl;
  o << TC::blue("   poy         ") << TC::red("  ww           ") << TC::orange("         zyz  ") << TC::magenta("  xvuvvv   vx   ") << TC::orange(" ywxyxyzywxxwwxz  ") << endl;
  o << TC::blue("   oo          ") << TC::red("  xw     zxy   ") << TC::orange(" yxz      zyz ") << TC::magenta("  xtv  wwwwzwvz ") << TC::orange(" zuuxyxxy         ") << endl;
  o << TC::blue("   oo          ") << TC::red("  zwx    wtw   ") << TC::orange(" zwx       yz ") << TC::magenta("  ywx  xtvy  wwz") << TC::orange("  xx              ") << endl;
  o << TC::blue("   tmntoooooopv") << TC::red("   ww   wwww   ") << TC::orange("  yxz      zzz") << TC::magenta("  zvw  zww   ww ") << TC::orange("  xx              ") << endl;
  o << TC::blue("    tmoooxxywxy") << TC::red("   ywz xwzxwz  ") << TC::orange("  zxyz     zzz") << TC::magenta("  zvw       zwx ") << TC::orange("  ww              ") << endl;
  o << TC::blue("               ") << TC::red("   zwx wx  ww  ") << TC::orange("  zz yyz   yy ") << TC::magenta("   ww       ywy ") << TC::orange("  xw              ") << endl;
  o << TC::blue("               ") << TC::red("    xwvw    wvy") << TC::orange("  yy  zyyzyz  ") << TC::magenta("   xx       xwz ") << TC::orange("  zwy             ") << endl;
  o << TC::blue("               ") << TC::red("     vtx       ") << TC::orange("         z    ") << TC::magenta("           zyz  ") << TC::orange("  xx              ") << endl;
  o << TC::blue("               ") << TC::red("               ") << TC::orange("              ") << TC::magenta("                ") << TC::orange("   xw             ") << endl;
  o << TC::blue("               ") << TC::red("               ") << TC::orange("              ") << TC::magenta("                ") << TC::orange("    xy            ") << endl;
  o << "                --- ChuMP! the CHUck Manager of Packages ---                  " << endl;

  return o.str();
}
