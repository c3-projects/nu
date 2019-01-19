#include "c3/nu/structs/dynamic.hpp"

using namespace c3::nu;

int main() {
  uint32_t a = 69420;
  std::string b = "foobar";
  data c = { 1, 2, 3, 4 };
  auto msg = squash_dynamic<uint16_t>(a, b, c);

  decltype(a) a_;
  decltype(b) b_;
  decltype(c) c_;
  expand_dynamic<uint16_t>(msg, a_, b_, c_);

  if (a != a_ || b != b_ || c != c_)
    throw std::runtime_error("Corruption detected!");
}
