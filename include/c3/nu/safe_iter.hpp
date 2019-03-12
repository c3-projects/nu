#pragma once

#include <iterator>
#include "c3/nu/moveable_ptr.hpp"

#include <vector>

namespace c3::nu {
  template<typename Iter>
  struct safe_iter_traits : public std::iterator_traits<Iter> {
    using iterator_category = typename std::forward_iterator_tag;
  };

  template<typename Iter>
  class safe_iter : public safe_iter_traits<Iter> {
  private:
    using traits = safe_iter_traits<Iter>;

  private:
    Iter base;
    Iter end;

  private:
    inline void incr() {
      if (base != end)
        ++base;
    }

  public:
    inline safe_iter& operator++() {
      incr();
      return *this;
    }
    inline safe_iter operator++(int) {
      auto cpy = *this;
      incr();
      return cpy;
    }
    inline typename traits::difference_type operator-(const safe_iter& other) {
      return base - other.base;
    }
    inline safe_iter& operator+=(typename traits::difference_type diff) {
      if (diff < 0)
        throw std::runtime_error("Cannot decrement safe iterators");

      if constexpr (std::is_same_v<typename traits::iterator_category, std::random_access_iterator_tag>) {
        auto dist_from_end = end - base;
        if (diff > dist_from_end)
          base = end;
        else
          base += dist_from_end;
      }
      else {
        for (; diff > 0 && base != end; --diff)
          incr();
      }
      return *this;
    }

    inline safe_iter operator[](typename traits::difference_type diff) {
      auto cpy = *this;
      return cpy += diff;
    }
    inline safe_iter operator+(typename traits::difference_type diff) {
      return (*this)[diff];
    }

  public:
    inline typename traits::reference operator*() {
      if (base == end)
        throw std::range_error("Tried to deref safe iterator out of range");
      else
        return *base;
    }
    inline typename traits::pointer operator->() {
      if (base == end)
        throw std::range_error("Tried to deref safe iterator out of range");
      else
        return base.operator->();
    }

  public:
    inline bool operator==(const safe_iter& other) const { return base == other.base; }
    inline bool operator!=(const safe_iter& other) const { return base != other.base; }
    inline bool operator<=(const safe_iter& other) const { return base <= other.base; }
    inline bool operator>=(const safe_iter& other) const { return base >= other.base; }
    inline bool operator< (const safe_iter& other) const { return base <  other.base; }
    inline bool operator> (const safe_iter& other) const { return base >  other.base; }

  public:
    inline bool is_end() { return base == end; }

  public:
    inline safe_iter(Iter base, Iter end) : base{base}, end{end} {}
    inline safe_iter() = default;
  };

  template<typename T>
  class safe {
  public:
    using iterator = safe_iter<typename T::iterator>;
    using const_iterator = safe_iter<typename T::const_iterator>;

  private:
    moveable_ptr<T> base = nullptr;

  public:
    iterator begin() { return { base->begin(), base->end() }; }
    iterator end() { return { base->end(), base->end() }; }
    iterator begin() const { return { base->begin(), base->end() }; }
    iterator end() const { return { base->end(), base->end() }; }
    const_iterator cbegin() const { return { base->cbegin(), base->cend() }; }
    const_iterator cend() const { return { base->cend(), base->cend() }; }

  public:
    inline safe(T& t) : base{&t} {}
  };
}
