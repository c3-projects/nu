#pragma once

#include <condition_variable>

#include "c3/nu/concurrency/mutexed.hpp"

namespace c3::nu {
  /// A bool that can be set from false to true, but not the other way around
  class gateway_bool {
  private:
    worm_mutexed<bool> _value = false;

    std::condition_variable _on_open;
    std::mutex _on_open_mutex;

  public:
    /// Checks if the gateway_bool has been opened
    inline bool is_open() const {
      return *_value.get_ro();
    }

    /// Sets the gateway to open
    inline void open() {
      *_value.get_rw() = true;
      _on_open.notify_all();
    }

    inline operator bool() const { return is_open(); }

    /// A section that, whilst not changing the value of the gateway,
    /// needs to be protected against it
    template<typename Func, typename Ret>
    inline Ret critical_section(Func f) {
      auto _ = _value.get_rw();
      return f();
    }

    /// IFF the value is not already true, calls set_func and sets the value to the result
    ///
    /// If the value is already true, set_func will not be called
    ///
    /// Returns the value of the gateway_bool, NOT whether set_func was called.
    template<typename SetFunc>
    inline bool maybe_open(SetFunc set_func) {
      static_assert(std::is_invocable_r_v<bool, SetFunc>,
                    "func must take no arguments, and return a result castable to bool");

      auto handle = _value.get_rw();
      if (*handle) return false;
      if (set_func()) {
        *handle = true;
        _on_open.notify_all();
      }

      return *handle;
    }

    /// IFF the value is not already true, calls set_func and sets the value to the result
    ///
    /// If the value is already true, already_set_func will be called instead of set_func
    ///
    /// Returns the value of the gateway_bool, NOT whether set_func was called.
    template<typename SetFunc, typename AlreadySetFunc>
    inline bool maybe_open(SetFunc set_func, AlreadySetFunc already_set_func) {
      static_assert(std::is_invocable_r_v<bool, SetFunc>,
                    "func must take no arguments, and return a result castable to bool");

      auto handle = _value.get_rw();
      if (*handle)
        already_set_func();
      else if (set_func()) {
        *handle = true;
        _on_open.notify_all();
      }
      return *handle;
    }

    /// Blocks until the gateway is open
    inline void wait_for_open() {
      std::unique_lock lock{_on_open_mutex};
      _on_open.wait(lock, [this] { return is_open(); } );
    }

    /// Blocks until the gateway is open and returns true,
    /// or returns false if the timeout is reached
    inline bool wait_for_open(timeout_t timeout) {
      std::unique_lock lock{_on_open_mutex};
      return _on_open.wait_for(lock, timeout, [this] { return is_open(); });
    }

    /// Returns true if the gateway was open, or was opened whilst waiting for the timeout,
    /// else returns false
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
}
