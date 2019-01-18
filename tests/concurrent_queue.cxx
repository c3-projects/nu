#include "c3/nu/concurrency/concurrent_queue.hpp"

using namespace c3::nu;
using namespace std::chrono_literals;

int main() {
  concurrent_queue<int> q;

  q.push(1, 10ms);
  q.pop();

  q.push(1, timeout_t{1});
  q.pop(timeout_t{1});
}
