#include "c3/nu/data/encoders/cstr.hpp"

#include <iostream>

using namespace c3::nu;

int main() {
  std::string str = "Foobar\177 a";
  str.push_back('\1');
  auto buf = cstr_encode(str);
  std::cout << buf << std::endl;
  if (cstr_decode(buf) != str)
    throw std::runtime_error("Failed to decode string");
}
