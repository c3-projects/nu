#include "c3/nu/concurrency/postbox.hpp"

using namespace c3::nu;

int main() {
  postbox<int, int> pb;

  auto lb_0 = pb.get(0);
  auto lb_1 = pb.get(1);

  pb.post(0, 69);
  pb.post(1, 420);
  pb.post(1, 180);

  if (lb_0->n_to_collect() != 1)
    throw std::runtime_error("Size corrupted");

  {
    auto lb_0_c = lb_0->collect_one();
    lb_0_c.wait();
    if (lb_0_c.try_take() != 69)
      throw std::runtime_error("lb0 corrupted");
  }

  {
    auto lb_1_c = lb_1->collect_all();
    if (lb_1_c.size() != 2 || lb_1_c[0] != 420)
      throw std::runtime_error("collect_all corrupted");
  }

}
