#include "c3/nu/data/structs/json.hpp"

#include <iostream>

using namespace c3::nu;

int main() {
  obj_struct ds;
  ds["foo"] = "bar";
  ds["qux"]["quux"] = "baz";
  ds["wibble"] = 5;
  ds["wobble"] = true;
  ds["420"] = 6.9;

  auto buf = json_encode(ds);

  std::cout << buf << std::endl;

  auto a = json_decode(buf);

  if (ds != a)
    throw std::runtime_error("Invalid buf");
}
