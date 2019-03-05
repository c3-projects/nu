#include "c3/nu/data/structs/json.hpp"

#include <iostream>

using namespace c3::nu;

int main() {
  obj_struct ds;
  ds["foo"] = "bar";
  ds["qux"]["quux"] = "baz";
  ds["wibble"] = 5;
  ds["wobble"] = true;
  ds["420"] = 0.5;

  auto buf = json_encode(ds);

  //std::cout << buf << std::endl;

  auto a = json_decode(buf);

  for (auto& i : ds)
    std::cout << json_encode(i.second) << std::endl;

  for (auto& i : a)
    std::cout << json_encode(i.second) << std::endl;

  if (ds != a)
    throw std::runtime_error("Invalid buf");
}
