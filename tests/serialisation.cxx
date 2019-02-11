#include "c3/nu/data.hpp"

using namespace c3::nu;

enum class enum_t {
  ValueA = 0,
  ValueB = 1
};

int main() {
  static_data<3> arr = {1, 2, 3};
  data arr_b(serialised_size<decltype(arr)>());
  serialise_static<decltype(arr)>(arr, arr_b);
  auto arr_ = deserialise<decltype(arr)>(arr_b);

  if (arr != arr_)
    throw std::runtime_error("Array serialisation corrupted");

  enum_t e = enum_t::ValueB;
  data e_b(serialised_size<decltype(e)>());
  serialise_static<decltype(e)>(e, e_b);
  auto e_ = deserialise<decltype(e)>(e_b);

  if (e != e_)
    throw std::runtime_error("Enum serialisation corrupted");
}
