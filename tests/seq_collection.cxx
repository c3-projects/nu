#include "c3/nu/data/collections/sequence.hpp"

using namespace c3::nu;

int main() {
  //throw std::runtime_error(typeid(remove_all<uint32_t>::type).name());

  static_assert (is_static_serialisable_v<uint32_t>);

  std::vector<int> a = { 420, 69, 180 };
  std::vector<std::string> b = { "foo", "bar", "baz", "quux", "wibble" };

  data buf_a = squash_seq(a.begin(), a.end());
  data buf_b = squash_seq<uint16_t>(b.begin(), b.end());

  decltype(a) a_ = expand_seq<int>(buf_a);
  decltype(b) b_ = expand_seq<std::string, uint16_t>(buf_b);

  if (a != a_)
    throw std::runtime_error("a");

  if (b != b_)
    throw std::runtime_error("b");
}
