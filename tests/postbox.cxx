#include "c3/nu/concurrency/postbox.hpp"

using namespace c3::nu;

int main() {
  postbox<int, int> pb;

  auto lb_0 = pb.get(0);
  auto lb_1 = pb.get(1);

  pb.post(0, 69);
  pb.post(1, 420);

  if (lb_0->n_to_collect() != 1)
    throw std::runtime_error("Size corrupted");

  lb_0->collect_one();
}
