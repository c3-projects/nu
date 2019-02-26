#include "c3/nu/concurrency/concurrent_queue.hpp"

using namespace c3::nu;
using namespace std::chrono_literals;

int main() {
  concurrent_queue<int> q;

  q.push(1);
  if (q.pop().try_take_final().value() != 1)
    throw std::runtime_error("First value corrupted");

  q.push(69);
  q.push(420);
  if (q.pop().try_take_final().value() != 69)
    throw std::runtime_error("Second value corrupted");

  if (q.pop().try_take_final().value() != 420)
    throw std::runtime_error("Third value corrupted");

  return 0;
}
