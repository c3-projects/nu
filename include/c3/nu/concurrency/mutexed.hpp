#pragma once

#include "c3/nu/concurrency/timeout.hpp"
#include "c3/nu/moveable_ptr.hpp"

#include <mutex>
#include <shared_mutex>

namespace c3::nu {
  /// A mutex wrapper that restricts access to a single requester at a time
  template<typename T>
  class mutexed {
  public:
    using type = T;

  private:
    std::timed_mutex _mutex = {};
    T _value;

  public:
    class handle {
    public:
      using type = T;
      using parent_t = moveable_ptr<mutexed<T>>;

    private:
      parent_t _parent;

    public:
      constexpr T& operator* () { return _parent->_value; }
      constexpr T* operator->() { return &_parent->_value; }
      constexpr const T& operator* () const { return _parent->_value; }
      constexpr const T* operator->() const { return &_parent->_value; }

    public:
      inline handle() : _parent{nullptr} {}
      inline handle(parent_t parent) : _parent{parent} {
        _parent->_mutex.lock();
      }
      inline handle(parent_t parent, timeout_t timeout) : _parent{parent} {
        if (!_parent->_mutex.try_lock_for(timeout))
          throw timed_out{};
      }
      inline ~handle() {
        if (_parent)
          _parent->_mutex.unlock();
      }
    };

  public:
    inline handle get_handle() { return {this}; }
    /// Tries to get a handle within the given timeout, otherwise throws timed_out
    inline handle get_handle(timeout_t timeout) { return {this, timeout}; }

    inline handle operator*() { return get_handle(); }

  public:
    template<typename... Args>
    inline mutexed(Args... args) : _value{std::forward<Args>(args)...} {}

    inline ~mutexed() { _mutex.lock(); _mutex.unlock(); }
  };

  /// A mutex wrapper that allows one writer or many readers concurrent access to a variable
  template<typename T>
  class worm_mutexed {
  public:
    using type = T;

  private:
    mutable std::shared_timed_mutex _mutex;
    T _value;

  public:
    class handle_ro {
    public:
      using type = T;
      using parent_t = moveable_ptr<const mutexed<T>>;

    private:
      parent_t _parent;

    public:
      constexpr const T& operator* () const { return _parent->_value; }
      constexpr const T* operator->() const { return &_parent->_value; }

    public:
      inline handle_ro() : _parent{nullptr} {}
      inline handle_ro(const parent_t parent) : _parent{parent} {
        _parent->_mutex.lock_shared();
      }
      inline handle_ro(const parent_t parent, timeout_t timeout) : _parent{parent} {
        if (!_parent->_mutex.try_lock_shared_for(timeout))
          throw timed_out{};
      }
      inline ~handle_ro() {
        if (_parent)
          _parent->_mutex.unlock_shared();
      }

      inline handle_ro(const handle_ro&) = delete;
      inline handle_ro(handle_ro&& other) : _parent{other} { other._parent = nullptr; }

      inline handle_ro& operator=(const handle_ro&) = delete;
      inline handle_ro& operator=(handle_ro&& other) {
        _parent = other._parent;
        other._parent = nullptr;
      }
    };

    class handle_rw {
    public:
      using type = T;
      using parent_t = moveable_ptr<mutexed<T>>;

    private:
      parent_t _parent;

    public:
      constexpr T& operator* () { return _parent->_value; }
      constexpr T* operator->() { return &_parent->_value; }
      constexpr const T& operator* () const { return _parent->_value; }
      constexpr const T* operator->() const { return &_parent->_value; }

    public:
      inline handle_rw() : _parent{nullptr} {}
      inline handle_rw(parent_t parent) : _parent{parent} {
        _parent->_mutex.lock();
      }
      inline handle_rw(parent_t parent, timeout_t timeout) : _parent{parent} {
        if (!_parent->_mutex.try_lock_for(timeout))
          throw timed_out{};
      }
      inline ~handle_rw() {
        if (_parent)
          _parent->_mutex.unlock();
      }
    };

  public:
    /// Gets a read-only handle
    handle_ro get_ro() const { return {this}; }
    /// Tries to get a read-only handle within the given timeout, otherwise throws timed_out
    handle_ro get_ro(timeout_t timeout) const { return {this, timeout}; }

    /// Gets a read/write handle
    handle_rw get_rw() { return {this}; }
    /// Tries to get a read/write handle within the given timeout, otherwise throws timed_out
    handle_rw get_rw(timeout_t timeout) { return {this, timeout}; }

    handle_rw operator*() { return get_rw(); }
    handle_ro operator*() const { return get_ro(); }

  public:
    template<typename... Args>
    inline worm_mutexed(Args... args) : _value{std::forward<Args>(args)...} {}

    inline ~worm_mutexed() { _mutex.lock(); _mutex.unlock(); }
  };
}
