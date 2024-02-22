
#include "manager.h"

Manager::Manager() {
  fetch = new Fetch();
}

optional<Package> Manager::getPackage(string name) {
  return fetch->fetch(name);
}

bool Manager::install(std::string packageName) {
  // need db file loaded

  // lookup package name (default to latest version)

  // get download url

  // fetch

  // validate

  // return true (maybe find better return value)
}


