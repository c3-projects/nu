#include "c3/nu/concurrency/postbox.hpp"

using namespace c3::nu;

int main() {
  postbox<int, int> pb;

  auto lb_0 = pb.get(0);
  auto lb_1 = pb.get(1);

  pb.post(0, 69);

  std::vector<int> to_post_1 = {420, 180};
  pb.post_all(1, to_post_1.begin(), to_post_1.end());

  if (lb_0->n_to_collect() != 1)
    throw std::runtime_error("Size corrupted");

  {
    auto lb_0_c = lb_0->collect();
    lb_0_c.wait();
    if (lb_0_c.try_take() != 69)
      throw std::runtime_error("lb0 corrupted");
  }

  {
    auto lb_1_res = lb_1->collect_all();
    if (lb_1_res != to_post_1)
      throw std::runtime_error("collect_all corrupted");
  }

}
