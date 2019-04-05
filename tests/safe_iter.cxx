#include "c3/nu/safe_iter.hpp"

#include <vector>

using namespace c3::nu;

int main() {
  {
    std::vector<int> yeet {1,2};

    auto begin = safe(yeet).begin();

    if (begin.is_end())
      throw std::runtime_error("safe iter set to end at start");

    if ((++begin).is_end())
      throw std::runtime_error("safe iter set to end after first incr");

    if (!(++begin).is_end())
      throw std::runtime_error("safe iter not set to end after second incr");

    try {
      *begin;
      throw std::runtime_error("Dereferencing at end did not throw an exception");
    }
    catch (std::range_error&) {}

    ++begin;

    try {
      *begin;
      throw std::runtime_error("Dereferencing after end did not throw an exception");
    }
    catch (std::range_error&) {}
  }

  {
    const std::vector<int> foo {1,2};
    auto begin = safe(foo).begin();

    if (begin.is_end())
      throw std::runtime_error("safe const iter set to end at start");

    if ((++begin).is_end())
      throw std::runtime_error("safe const iter set to end after first incr");

    if (!(++begin).is_end())
      throw std::runtime_error("safe const iter not set to end after second incr");

    try {
      *begin;
      throw std::runtime_error("Dereferencing at const end did not throw an exception");
    }
    catch (std::range_error&) {}

    ++begin;

    try {
      *begin;
      throw std::runtime_error("Dereferencing after const end did not throw an exception");
    }
    catch (std::range_error&) {}
  }

  return 0;
}

