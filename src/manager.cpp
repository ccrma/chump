
#include "manager.h"

Manager::Manager() {
  fetch = new Fetch();
}

Package Manager::getPackage(std::string name) {
  return fetch->fetch(name);
}


