#include "c3/nu/concurrency/concurrent_queue.hpp"

using namespace c3::nu;
using namespace std::chrono_literals;

int main() {
  concurrent_queue<int> q;

  q.push(1);
  q.pop();

  q.push(1);
  q.pop().wait_final(1s);
}
