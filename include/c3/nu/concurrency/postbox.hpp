#include <thread>
#include <map>
#include <optional>

#include "c3/nu/concurrency/mutexed.hpp"
#include "c3/nu/concurrency/concurrent_queue.hpp"
#include "c3/nu/concurrency/cancellable.hpp"

namespace c3::nu {
  template<typename T>
  class letterbox;

  template<typename T>
  class _postbox_manager {
  public:
    _postbox_manager(const _postbox_manager&) = delete;
    _postbox_manager& operator=(const _postbox_manager&) = delete;
  };

  template<typename Id, typename Value>
  class postbox {
    //friend class letterbox<Value>;
  private:
    worm_mutexed<std::map<Id, std::shared_ptr<letterbox<Value>>>> _boxes;

  public:
    inline std::shared_ptr<letterbox<Value>> get(Id id) {
      auto handle = _boxes.get_rw();
      auto iter = handle->find(id);
      if (iter == handle->end())
        return handle->emplace(id, std::make_shared<letterbox<Value>>());
      else
        return *iter;
    }
    inline void remove(Id id) {
      auto handle = _boxes.get_rw();
      handle.erase(id);
    }
  public:
    inline void post(Id id, Value v) {
      get(id)->post(v);
    }

  public:
    inline std::shared_ptr<letterbox<Value>> operator[](Id id) { return get(id); }
  };

  template<typename T>
  class letterbox {
  private:
    concurrent_queue<T> _box;
  public:
    inline void post(T t) {
      _box.push(std::move(t));
    }

    inline T collect_one() {
      return _box.pop();
    }

    inline std::vector<T> collect_all() {
      std::vector<T> ret;
      while (auto x = _box.try_pop())
        ret.emplace(std::move(x));
    }

    inline size_t n_to_collect() const {
      return _box.size();
    }
  };
}
