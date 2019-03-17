#include "c3/nu/data/collections/static.hpp"

using namespace c3::nu;

int main() {
  //throw std::runtime_error(typeid(remove_all<uint32_t>::type).name());
;
  uint32_t b = 0x4a;
  char d = '\\';
  static_data<2> e = {9, 9};

  data buf = squash_static(b, d, e);

  decltype(b) b_;
  decltype(d) d_;
  decltype(e) e_;

  expand_static(buf, b_, d_, e_);

  if (b != b_)
    throw std::runtime_error("b");

  if (d != d_)
    throw std::runtime_error("d");

  if (e != e_)
    throw std::runtime_error("e");
}
