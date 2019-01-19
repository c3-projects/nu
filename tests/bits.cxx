#include "c3/nu/bits.hpp"
#include "c3/nu/data.hpp"

using namespace c3::nu;

template<n_bits_rep_t Size>
auto check_one() {
  data msg = serialise("hi");

  auto split_val = bit_datum<Size>::split(msg);

  data combined_val = bit_datum<Size>::combine(split_val);
  combined_val.resize(msg.size());

  if (combined_val != msg)
    throw std::runtime_error("combine(split(msg)) != msg");
}

int main() {
  check_one<1>();
  check_one<2>();
  check_one<5>();
  check_one<7>();
  check_one<8>();
}
