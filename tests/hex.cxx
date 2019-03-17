#include "c3/nu/data/encoders/hex.hpp"
#include "c3/nu/data.hpp"

using namespace c3::nu;

#include <iostream>

int main() {
  std::string str = "Hello, world!";
  std::string hex = hex_encode(str);
  std::string str_ = hex_decode<std::string>(hex);

  if (str != str_)
    throw std::runtime_error("Base64 data corrupted!");

  return 0;
}
