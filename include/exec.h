#ifndef __EXEC_H__
#define __EXEC_H__

#include "chuck_version.h"
#include "manager.h"

std::string printLogo();
void printPackage(Package pkg);
void printPackages(Manager *mgr);

void printAllPackages(Manager *mgr);
void printInstalledPackages(Manager *mgr);
void printPackagesMultiLine(Manager *mgr, bool print_installed,
                            bool print_updates);

// https://stackoverflow.com/questions/19580877/how-to-truncate-a-string-formating-c
std::string truncate(std::string str, size_t width, bool show_ellipsis = true);

// Function used in the `chump list` command
void printPackages(Manager *mgr, bool print_installed, bool print_updates);

// Check if there are any out of date packages, return true if so
bool updateAvailable(Manager *mgr);

void printPackagesMultiLine(Manager *mgr, bool print_installed,
                            bool print_updates);

// Print all packages, regardless of whether they're installed or not
void printAllPackages(Manager *mgr);

// Print only installed packages (and relevant information such as install path)
void printInstalledPackages(Manager *mgr);

void printPackage(Manager *mgr, Package p);

void get_terminal_size(int &width, int &height);

void clear();

std::string jumbleColors(std::string str, int counter);

std::string riverColors(std::string str, int counter);

std::string dimColors(std::string str, int counter);

std::string bedtimeColors(std::string str, int counter);

std::string bedtimeLogo(int counter);

std::string dimLogo(int counter);

std::string riverLogo(int counter);

std::string jumbleLogo(int counter);

std::string printLogo();

#endif
