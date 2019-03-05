#include "c3/nu/bigint.hpp"

using namespace c3::nu;

int main() {
  bigint a = 420;
  bigint b = 69;

  if (static_cast<int>(a += b) != (420 + 69))
    throw std::runtime_error("Failed to += values");

  if (static_cast<int>(a -= b) != (420))
    throw std::runtime_error("Failed to -= values");

  if (static_cast<int>(a + b) != (420 + 69))
    throw std::runtime_error("Failed to + values");

  if (static_cast<int>(a - b) != (420 - 69))
    throw std::runtime_error("Failed to - values");

  {
    auto i = b - a;
    auto j = static_cast<int>(i);
    if (j != (69 - 420))
      throw std::runtime_error("Failed to - to neg values");
  }

  return 0;
}
