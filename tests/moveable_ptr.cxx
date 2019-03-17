#pragma once

#include "c3/nu/moveable_ptr.hpp"

#include <stdexcept>

using namespace c3::nu;

int main() {
  int a = 5;
  int b = 10;

  moveable_ptr<int> ptr = &a;

  if (*ptr != a)
    throw std::runtime_error("Pointer did not construct as it was told");

  ptr = &b;

  if (*ptr != b)
    throw std::runtime_error("Pointer did not change as it was told");

  moveable_ptr<int> _ptr = std::move(ptr);

  if (*_ptr != b)
    throw std::runtime_error("Pointer did not move to value");

  if (*ptr != nullptr)
    throw std::runtime_error("Pointer did not move from value");
}
