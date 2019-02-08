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
    std::shared_ptr<letterbox<Value>> allocate(Id id) {
      auto handle = _boxes.get_rw();
      handle.empace(id, std::make_shared<letterbox<Value>>());
    }
  };


  template<typename T>
  class letterbox {
  private:
    concurrent_queue<T> _box;
  public:
    void post(T t) {
      _box.push(std::move(t));
    }
    T collect_one() {
      return _box.pop();
    }
    /*
    std::vector<T> collect() {
      std::vector<T> ret;
      while(_box.pop());
    }*/
  };
}
