#include "c3/nu/concurrency/concurrent_map.hpp"

using namespace c3::nu;

int main() {
  concurrent_map<uint64_t, std::string> m;

  m.emplace(0, "Foo");
  m[1] = "Bar";

  if (m.at(0) != std::string{"Foo"})
    throw std::runtime_error("First failed");

  if (!m.contains(1))
    throw std::runtime_error("Second failed");

  return 0;
}
