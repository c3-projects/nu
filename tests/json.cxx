#include "c3/nu/data/structs/json.hpp"

using namespace c3::nu;

int main() {
  data_struct ds;
  ds["foo"] = "bar";
  ds["qux"]["quux"] = "baz";
}
