#ifndef __UNINSTALLER_H__
#define __UNINSTALLER_H__

#include "package.h"
#include "package_list.h"

using std::optional;

namespace fs = std::filesystem;

// Abstract base class
class UninstallerBase {
public:
  virtual ~UninstallerBase() = default;
  virtual bool uninstall(std::string package_name) = 0;
};


class Uninstaller : public UninstallerBase {
 public:
  Uninstaller(PackageList* package_list);

 public:
  bool uninstall(std::string package_name);

 private:
  PackageList* package_list;
};

#endif
