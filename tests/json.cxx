#include "c3/nu/data/structs/json.hpp"

#include <iostream>

using namespace c3::nu;

int main() {
  data_struct ds;
  ds["foo"] = "bar";
  ds["qux"]["quux"] = "baz";

  std::cout << json_encode(ds) << std::endl;
}
