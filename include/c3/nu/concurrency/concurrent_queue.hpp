#pragma once

#include <queue>
#include <thread>
#include <optional>

#include "c3/nu/concurrency/cancellable.hpp"

namespace c3::nu {
  template<typename T>
  class concurrent_queue {
  private:
    std::queue<T> _base;
    std::queue<cancellable_provider<T>> _requesters;
    mutable std::mutex _mutex;

  private:
    inline void _push_one(T t) {
      while (_requesters.size() > 0) {
        auto req = _requesters.front();
        _requesters.pop();
        if (req.provide(t))
          return;
      }

      _base.emplace(std::move(t));
    }

  public:
    inline cancellable<T> pop() {
      std::unique_lock lock{_mutex};
      if (_base.size() > 0) {
        auto ret = _base.front();
        _base.pop();
        return { std::move(ret) };
      }
      else {
        auto requester = _requesters.emplace();
        return requester.get_cancellable();
      }
    }

    inline std::optional<T> try_pop() {
      std::unique_lock lock{_mutex};
      if (_base.size() > 0) {
        auto ret = _base.front();
        _base.pop();
        return { std::move(ret) };
      }
      else
        return std::nullopt;
    }

    inline std::vector<T> pop_all() {
      std::unique_lock lock{_mutex};

      std::vector<T> ret;

      for (auto iter = ret.rbegin(); iter != ret.rend(); ++iter)
        ret.emplace_back(std::move(*iter));

      _base = decltype(_base){};

      return ret;
    }

    inline void push(T t) {
      std::unique_lock lock{_mutex};

      _push_one(std::move(t));
    }

    template<typename Iter>
    inline void push_all(Iter begin, Iter end) {
      std::unique_lock lock{_mutex};

      for (auto iter = begin; iter != end; ++iter)
        _push_one(*iter);
    }

    template<typename Iter>
    inline void move_all(Iter begin, Iter end) {
      std::unique_lock lock{_mutex};

      for (auto iter = begin; iter != end; ++iter)
        _push_one(std::move(*iter));
    }

    inline size_t size() const {
      std::unique_lock lock{_mutex};
      return _base.size();
    }
  };
}
