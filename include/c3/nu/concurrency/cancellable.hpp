#pragma once

#include <optional>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
#include <condition_variable>
#include <future>

#include "c3/nu/concurrency/timeout.hpp"
#include "c3/nu/concurrency/mutexed.hpp"

namespace c3::nu {
  /// A bool that can be set from false to true, but not the other way around
  class gateway_bool {
  private:
    worm_mutexed<bool> _value = false;

    std::condition_variable _on_open;
    std::mutex _on_open_mutex;

  public:
    inline bool is_open() const {
      return *_value.get_ro();
    }

    inline void open() {
      *_value.get_rw() = true;
      _on_open.notify_all();
    }

    inline operator bool() const { return is_open(); }

    /// IFF the value is not already true, calls func and sets the value to the result
    ///
    /// If the value is already true, func will not be called
    ///
    /// Returns the value of the gateway_bool, NOT whether func was called.
    template<typename SetFunc>
    inline bool maybe_open(SetFunc set_func) {
      static_assert(std::is_invocable_r_v<bool, SetFunc>,
                    "func must take no arguments, and return a result castable to bool");

      auto handle = _value.get_rw();
      if (*handle) return false;
      // This cannot move from true->false, as we checked that it wasn't true
      *handle = set_func();

      return *handle;
    }

    template<typename SetFunc, typename AlreadySetFunc>
    inline bool maybe_open(SetFunc set_func, AlreadySetFunc already_set_func) {
      static_assert(std::is_invocable_r_v<bool, SetFunc>,
                    "func must take no arguments, and return a result castable to bool");

      auto handle = _value.get_rw();
      if (*handle)
        already_set_func();
      else
        // This cannot move from true->false, as we checked that it wasn't true
        *handle = set_func();
      return *handle;
    }

    inline void wait_for_open() {
      std::unique_lock lock{_on_open_mutex};
      _on_open.wait(lock, [this] { return is_open(); } );
    }

    inline bool wait_for_open(timeout_t timeout) {
      std::unique_lock lock{_on_open_mutex};
      return _on_open.wait_for(lock, timeout, [this] { return is_open(); });
    }

    /// Returns true if the gateway was open, or was opened whilst waiting fo the timeout,
    /// or returns false
    inline bool open_after(timeout_t timeout) {
      std::unique_lock lock{_on_open_mutex};

      if(_on_open.wait_for(lock, timeout, [&] { return is_open(); }))
        return true;

      // Lock again to make sure we didn't miss it
      auto handle = _value.get_rw();

      if (*handle)
        return true;
      else {
        *handle = true;
        return false;
      }
    }
  };

  enum class cancellable_state {
    Provided,
    Cancelled,
    Empty
  };

  template<typename T>
  class _cancellable_shared_state {
  public:
    std::optional<T> result = std::nullopt;

    gateway_bool final_state_decided;

  public:
    bool is_cancelled() const {
      return final_state_decided && !result.has_value();
    }

    inline cancellable_state get_state() const {
      if (!final_state_decided) return cancellable_state::Empty;
      if (result) return cancellable_state::Provided;
      else return cancellable_state::Cancelled;
    }
  };

  template<typename T>
  class cancellable_provider;

  template<typename T>
  class cancellable {
    friend cancellable_provider<T>;

  private:
    std::shared_ptr<_cancellable_shared_state<T>> shared_state;

  public:
    std::optional<T> get_or_cancel() {
      // Cancel if not set
      shared_state->final_state_decided.maybe_open([&] { return true; });

      return shared_state->result;
    }

    std::optional<T> get_or_cancel(timeout_t timeout) {
      shared_state->final_state_decided.open_after(timeout);
      return shared_state->result;
    }

    inline bool is_cancelled() { return shared_state->is_cancelled(); }

    inline std::optional<T> try_get() {
      if (shared_state->final_state_decided)
        return shared_state->result;
      else
        return std::nullopt;
    }

    inline cancellable_state get_state() const { return shared_state->get_state(); }

  private:
    inline cancellable(decltype(shared_state) shared_state) : shared_state{shared_state} {}
  };

  template<typename T>
  class cancellable_provider {
  private:
    std::shared_ptr<_cancellable_shared_state<T>> shared_state =
      std::make_shared<_cancellable_shared_state<T>>();

  public:
    template<typename Func>
    inline cancellable_state maybe_provide(Func func) {
      cancellable_state ret;

      shared_state->final_state_decided.maybe_open([&] {
        try {
          auto result = func();

          if (result) {
            shared_state->result = std::move(result);
            ret = cancellable_state::Provided;

            return true;
          }
          else {
            ret = cancellable_state::Empty;
            return false;
          }
        } catch(...) {
          ret = cancellable_state::Cancelled;
          return true;
        }
      });

      return ret;
    }

    inline bool is_cancelled() { return shared_state->is_cancelled(); }

    inline cancellable_state get_state() const { return shared_state->get_state(); }

    inline void cancel() { shared_state->final_state_decided.open(); }

  public:
    inline cancellable<T> get_cancellable() { return { shared_state }; }
  };
}
