#pragma once

#include "c3/nu/concurrency/timeout.hpp"

#include <mutex>
#include <shared_mutex>

namespace c3::nu {
  template<typename T>
  class mutexed {
  private:
    std::timed_mutex _mutex = {};
    T _value;

  public:
    class handle {
    private:
      mutexed<T>* _parent;

    public:
      T& operator* () { return _parent->_value; }
      T* operator->() { return &_parent->_value; }
      const T& operator* () const { return _parent->_value; }
      const T* operator->() const { return &_parent->_value; }

    public:
      inline handle() : _parent{nullptr} {}
      inline handle(mutexed<T>* parent) : _parent{parent} {
        _parent->_mutex.lock();
      }
      inline handle(mutexed<T>* parent, timeout_t timeout) : _parent{parent} {
        if (!_parent->_mutex.try_lock_for(timeout))
          throw timed_out{};
      }
      inline ~handle() {
        if (_parent)
          _parent->_mutex.unlock();
      }

      inline handle(const handle&) = delete;
      inline handle(handle&& other) : _parent{other} { other._parent = nullptr; }

      inline handle& operator=(const handle&) = delete;
      inline handle& operator=(handle&& other) {
        _parent = other._parent;
        other._parent = nullptr;
      }
    };

  public:
    inline handle get_handle() { return {this}; }
    inline handle get_handle(timeout_t timeout) { return {this, timeout}; }

    inline handle operator*() { return get_handle(); }

  public:
    template<typename... Args>
    inline mutexed(Args... args) : _value{std::forward<Args>(args)...} {}

    inline ~mutexed() { _mutex.lock(); _mutex.unlock(); }
  };

  template<typename T>
  class worm_mutexed {
  private:
    mutable std::shared_timed_mutex _mutex;
    T _value;

  public:
    class handle_ro {
    private:
      const worm_mutexed<T>* _parent;

    public:
      const T& operator* () const { return _parent->_value; }
      const T* operator->() const { return &_parent->_value; }

    public:
      inline handle_ro() : _parent{nullptr} {}
      inline handle_ro(const worm_mutexed<T>* parent) : _parent{parent} {
        _parent->_mutex.lock_shared();
      }
      inline handle_ro(const worm_mutexed<T>* parent, timeout_t timeout) : _parent{parent} {
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
    private:
      worm_mutexed<T>* _parent;

    public:
      T& operator* () { return _parent->_value; }
      T* operator->() { return &_parent->_value; }
      const T& operator* () const { return _parent->_value; }
      const T* operator->() const { return &_parent->_value; }

    public:
      inline handle_rw() : _parent{nullptr} {}
      inline handle_rw(worm_mutexed<T>* parent) : _parent{parent} {
        _parent->_mutex.lock();
      }
      inline handle_rw(worm_mutexed<T>* parent, timeout_t timeout) : _parent{parent} {
        if (!_parent->_mutex.try_lock_for(timeout))
          throw timed_out{};
      }
      inline ~handle_rw() {
        if (_parent)
          _parent->_mutex.unlock();
      }

      inline handle_rw(const handle_ro&) = delete;
      inline handle_rw(handle_rw&& other) : _parent{other} { other._parent = nullptr; }

      inline handle_rw& operator=(const handle_rw&) = delete;
      inline handle_rw& operator=(handle_rw&& other) {
        _parent = other._parent;
        other._parent = nullptr;
      }
    };

  public:
    handle_ro get_ro() const { return {this}; }
    handle_ro get_ro(timeout_t timeout) const { return {this, timeout}; }

    handle_rw get_rw() { return {this}; }
    handle_rw get_rw(timeout_t timeout) { return {this, timeout}; }

    handle_rw operator*() { return get_rw(); }
    handle_ro operator*() const { return get_ro(); }

  public:
    template<typename... Args>
    inline worm_mutexed(Args... args) : _value{std::forward<Args>(args)...} {}

    inline ~worm_mutexed() { _mutex.lock(); _mutex.unlock(); }
  };
}
