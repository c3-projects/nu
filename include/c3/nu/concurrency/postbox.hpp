#include <thread>
#include <map>
#include <optional>

#include "c3/nu/concurrency/mutexed.hpp"
#include "c3/nu/concurrency/concurrent_queue.hpp"
#include "c3/nu/concurrency/cancellable.hpp"

namespace c3::nu {
  template<typename T>
  class letterbox;

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
        return handle->emplace(id, std::make_shared<letterbox<Value>>()).first->second;
      else
        return iter->second;
    }
    inline void remove(Id id) {
      auto handle = _boxes.get_rw();
      handle.erase(id);
    }
  public:
    inline void post(Id id, Value v) {
      get(id)->post(v);
    }

    template<typename Iter>
    inline void post_all(Id id, Iter begin, Iter end) {
      get(id)->post_all(begin, end);
    }

    template<typename Iter>
    inline void move_all(Id id, Iter begin, Iter end) {
      get(id)->move_all(begin, end);
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

    template<typename Iter>
    inline void post_all(Iter begin, Iter end) {
      _box.push_all(begin, end);
    }

    template<typename Iter>
    inline void move_all(Iter begin, Iter end) {
      _box.move_all(begin, end);
    }

    inline cancellable<T> collect() {
      return _box.pop();
    }

    inline std::vector<T> collect_all() {
      std::vector<T> ret;
      while (auto x = _box.try_pop())
        ret.emplace_back(std::move(*x));
      return ret;
    }

    inline size_t n_to_collect() const {
      return _box.size();
    }
  };
}
