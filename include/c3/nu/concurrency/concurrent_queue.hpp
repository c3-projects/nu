#pragma once

#include <queue>
#include <thread>
#include <optional>

#include "c3/nu/concurrency/mutexed.hpp"

namespace c3::nu {
  template<typename T>
  class concurrent_queue {
  private:
    worm_mutexed<std::queue<T>> _base;

  public:
    inline T pop() {
      while (true) {
        auto handle = *_base;
        if (handle->size() == 0) { std::this_thread::yield(); continue; }
        T ret = std::move(handle->front());
        handle->pop();
        return ret;
      }
    }
    inline std::optional<T> try_pop() {
      auto handle = *_base;
      if (handle->size() == 0)
        return std::nullopt;
      T ret = std::move(handle->front());
      handle->pop();
      return { std::move(ret) };
    }
    inline T pop(timeout_t timeout) {
      auto deadline = now() + timeout;

      while (true) {
        // Will throw timed_out if we run out of time
        auto handle = _base.get_rw(deadline - now());
        if (handle->size() == 0) { std::this_thread::yield(); continue; }
        T ret = std::move(handle->front());
        handle->pop();
        return ret;
      }
    }
    inline std::optional<T> try_pop(timeout_t timeout) {
      // Will throw timed_out if we run out of time
      auto handle = _base.get_rw(timeout);
      if (handle->size() == 0)
        return std::nullopt;
      T ret = std::move(handle->front());
      handle->pop();
      return { std::move(ret) };
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
