#pragma once

#include <utility>

namespace c3::nu {
  /// Nulls the pointer on move
  ///
  /// No other smart pointer semantics exist in this type
  ///
  /// Useful for a mutable reference to a parent
  template<typename T>
  class moveable_ptr {
  public:
    using pointer = T*;
    using element_type = T;

  private:
    pointer ptr;

  public:
    constexpr operator pointer&() { return ptr; }
    constexpr operator const pointer&() const { return ptr; }
    constexpr pointer operator->() { return ptr; }
    constexpr const pointer operator->() const { return ptr; }
    constexpr element_type& operator*() { return *ptr; }
    constexpr const element_type& operator*() const { return *ptr; }
    constexpr pointer get() { return ptr; }
    constexpr pointer release() { auto ret = ptr; ptr = nullptr; return ret; }
    constexpr inline void swap(moveable_ptr& other) { std::swap(ptr, other.ptr); }

  public:
    constexpr moveable_ptr(pointer p) : ptr{p} {}

  public:
    constexpr moveable_ptr(const moveable_ptr&) = delete;
    constexpr moveable_ptr(moveable_ptr&& other) : ptr{other.release()} {}

    constexpr moveable_ptr& operator=(const moveable_ptr&) = delete;
    constexpr moveable_ptr& operator=(moveable_ptr&& other) {
      ptr = other.release();
    }
  };
}
