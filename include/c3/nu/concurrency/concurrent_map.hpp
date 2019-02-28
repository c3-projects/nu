#pragma once

#include <map>
#include "c3/nu/concurrency/mutexed.hpp"
#include "c3/nu/moveable_ptr.hpp"

namespace c3::nu {
  template<typename Key, typename Value>
  class concurrent_map {
  public:
    using base_t = std::map<Key, Value>;
    using key_t = Key;
    using value_t = Value;
    using iterator_t = typename base_t::iterator;

  private:
    worm_mutexed<base_t> _base;

  public:
    template<typename... Args>
    inline auto emplace(Args... args) {
      return _base.get_rw()->emplace(std::forward<Args>(args)...);
    };
    template<typename... Args>
    inline auto insert(Args... args) {
      return _base.get_rw()->insert(std::forward<Args>(args)...);
    };
    template<typename... Args>
    inline auto insert_or_assign(Args... args) {
      return _base.get_rw()->insert_or_assign(std::forward<Args>(args)...);
    };
    template<typename... Args>
    inline auto erase(Args... args) {
      return _base.get_rw()->erase(std::forward<Args>(args)...);
    };
    inline bool contains(const Key& key) const {
      auto handle = _base.get_ro();
      auto iter = handle->find(key);
      return iter != handle->end();
    }
    template<typename... Args>
    inline auto at(Args... args) const {
      return _base.get_ro()->at(std::forward<Args>(args)...);
    }

    template<typename Arg>
    inline auto operator[](Arg arg) {
      return _base.get_rw()->operator[](std::forward<Arg>(arg));
    }

  public:
    template<typename... Args>
    inline concurrent_map(Args... args) : _base(std::forward<Args>(args)...) {}
  };
}
