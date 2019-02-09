#include "c3/nu/structs/mixed.hpp"

using namespace c3::nu;

int main() {
  //throw std::runtime_error(typeid(remove_all<uint32_t>::type).name());

  static_assert (is_static_serialisable_v<uint32_t>);

  std::string a = "foobar";
  uint32_t b = 0x4a;
  data c = {69, 180};
  char d = '\\';

  data buf = squash<uint16_t>(a, b, c, d);

  decltype(a) a_;
  decltype(b) b_;
  decltype(c) c_;
  decltype(d) d_;

  expand<uint16_t>(buf, a_, b_, c_, d_);

  if (a != a_)
    throw std::runtime_error("a");

  if (b != b_)
    throw std::runtime_error("b");

  if (c != c_)
    throw std::runtime_error("c");
}
