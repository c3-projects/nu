#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>

#include "c3/nu/concurrency/mutexed.hpp"

namespace c3::nu {
  /// A bool that can be set from false to true once, but not the other way around
  class gateway_bool {
  private:
    // Since we use mutexes, we do not need this to be atmic
    bool _value = false;
    std::mutex _mutex;
    std::condition_variable _on_open;

  public:
    /// Checks if the gateway_bool has been opened
    inline bool is_open() const {
      return _value;
    }

    /// Sets the gateway to open
    inline void open() {
      std::scoped_lock lock{_mutex};
      _value = true;
      _on_open.notify_all();
    }

    inline operator bool() const { return is_open(); }

    /// A section that, whilst not changing the value of the gateway,
    /// needs to be protected against it
    template<typename Func>
    inline void critical_section(Func f) {
      std::scoped_lock lock{_mutex};
      f();
    }

    /// IFF the value is not already true, calls set_func and sets the value to the result
    ///
    /// If the value is already true, set_func will not be called
    ///
    /// Returns the value of the gateway_bool, NOT whether set_func was called.
    inline bool maybe_open(std::function<bool()> set_func) {
      std::scoped_lock lock{_mutex};

      if (!_value && set_func()) {
        _value = true;
        _on_open.notify_all();
      }

      return _value;
    }

    /// IFF the value is not already true, calls set_func and sets the value to the result
    ///
    /// If the value is already true, already_set_func will be called instead of set_func
    ///
    /// Returns the value of the gateway_bool, NOT whether set_func was called.
    template<typename AlreadySetFunc>
    inline bool maybe_open(std::function<bool()> set_func, AlreadySetFunc already_set_func) {
      std::scoped_lock lock{_mutex};

      if (_value) already_set_func();
      else if (set_func()) {
        _value = true;
        _on_open.notify_all();
      }

      return _value;
    }

    /// Blocks until the gateway is open
    inline void wait_for_open() {
      std::unique_lock lock{_mutex};
      _on_open.wait(lock, [this] { return is_open(); } );
    }

    /// Blocks until the gateway is open and returns true,
    /// or returns false if the timeout is reached
    inline bool wait_for_open(timeout_t timeout) {
      std::unique_lock lock{_mutex};
      return _on_open.wait_for(lock, timeout, [this] { return is_open(); });
    }

    /// Opens the gatway by timeout, returning as soon as it is opened
    ///
    /// Returns true if the gateway was open, or was opened whilst waiting for the timeout,
    /// else returns false
    inline bool open_after(timeout_t timeout) {
      if(wait_for_open(timeout))
        return true;
      else {
        // Lock again to make sure we didn't miss it
        std::scoped_lock lock{_mutex};
        if (_value) return true;
        else {
          _value = true;
          _on_open.notify_all();
          return false;
        }
      }
    }
  };
}
