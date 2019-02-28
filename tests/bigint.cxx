#include "c3/nu/bigint.hpp"

using namespace c3::nu;

int main() {
  biguint a = 420;
  biguint b = 69;

  if (static_cast<int>(a += b) != (420 + 69))
    throw std::runtime_error("Failed to += values");

  if (static_cast<int>(a -= b) != (420))
    throw std::runtime_error("Failed to -= values");

  if (static_cast<int>(a + b) != (420 + 69))
    throw std::runtime_error("Failed to + values");

  if (static_cast<int>(a - b) != (420 - 69))
    throw std::runtime_error("Failed to + values");

  return 0;
}
