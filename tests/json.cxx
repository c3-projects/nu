#include "c3/nu/data/encoders/json.hpp"

#include <iostream>
#include <fstream>

using namespace c3::nu;

int main() {
  obj_struct ds;
  ds["foo"] = "bar";
  ds["qux"]["quux"] = "baz";
  ds["wibble"] = 5;
  ds["wobble"] = true;
  ds["420"] = 0.5;
  ds["arr"].as<obj_struct::arr_t>() = { -1, 2, 3, 4 };
  ds["empty_arr"].as<obj_struct::arr_t>();
  ds["empty"] = nullptr;

  auto buf = json_encode(ds);

  auto _ds = json_decode(buf);

  auto _buf = json_encode(_ds);

  if (ds != _ds)
    throw std::runtime_error("Invalid buf");

  /*
  std::ifstream ifs("testfiles/fuzz.json");
  std::string line;
  while (std::getline(ifs, line)) {
    try { json_decode(line); }
    catch (std::exception& e) {
      std::cout << line << std::endl;
      std::cerr << e.what() << std::endl;
    }
  }
  */
}

