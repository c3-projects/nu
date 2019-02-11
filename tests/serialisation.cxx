#include "c3/nu/data.hpp"

using namespace c3::nu;

int main() {
  static_data<3> arr = {1, 2, 3};
  data arr_b(serialised_size<decltype(arr)>());
  serialise_static<decltype(arr)>(arr, arr_b);
  auto arr_ = deserialise<decltype(arr)>(arr_b);

  if (arr != arr_)
    throw std::runtime_error("Array serialisation corrupted");
}
