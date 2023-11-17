#ifndef __FETCH_H__
#define __FETCH_H__

#include "package.h"
#include <nlohmann/json.hpp>

#include <fstream>
using json = nlohmann::json;

// Abstract base class
class FetchBase {
public:
    virtual ~FetchBase() = default;
    virtual Package fetch(std::string data) = 0;
};


class Fetch : public FetchBase {
 public:
  /* Fetch(); */
  Package fetch(std::string data);
};


#endif
