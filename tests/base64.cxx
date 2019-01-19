#include "c3/nu/data/base64.hpp"

using namespace c3::nu;

#include <iostream>

int main() {
  std::string str = "Hello, world!";

  std::string str_ = base64_decode<std::string>(base64_encode(str));

  if (str != str_)
    throw std::runtime_error("Base64 data corrupted!");

  return 0;
}
