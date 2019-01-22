#pragma once

#include <queue>
#include <thread>
#include <optional>

#include "c3/nu/concurrency/cancellable.hpp"

namespace c3::nu {
  // Condvars are too fiddly for me to get to work in this case
  // TODO: move to a condvar-based system rather than a poll-based one
  template<typename T>
  class concurrent_queue {
  private:
    worm_mutexed<std::queue<T>> _base;

  public:
    inline cancellable<T> pop() {
      cancellable_provider<T> provider;

      std::thread([=]() mutable {
        do {
          auto handle = _base.get_rw();
          if (handle->size() == 0) { std::this_thread::yield(); continue; }
          provider.maybe_provide([&] {
            T ret = std::move(handle->front());
            handle->pop();
            return ret;
          });
        }
        while (!provider.is_decided());
      }).detach();

      return provider.get_cancellable();
    }

    inline void push(T t) {
      auto handle = *_base;
      handle->emplace(std::move(t));
    }

    inline void push(T t, timeout_t timeout) {
      auto deadline = now() + timeout;

      // Will throw timed_out if we run out of time
      auto handle = _base.get_rw(deadline - now());
      handle->emplace(std::move(t));
    }

    inline size_t size() const { return (*_base)->size(); }
  };
}
