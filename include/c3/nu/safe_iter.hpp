#pragma once

#include <iterator>
#include "c3/nu/moveable_ptr.hpp"
#include "c3/nu/sfinae.hpp"

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
    inline Iter& get_base() { return base; }
    inline Iter& get_end() { return end; }
    inline const Iter& get_base() const { return base; }
    inline const Iter& get_end() const { return end; }

  public:
    inline safe_iter(Iter base, Iter end) : base{base}, end{end} {}
    inline safe_iter() = default;
  };

  template<typename T, typename = void>
  class safe {
  public:
    using iterator = safe_iter<iterator_t<T>>;
    using const_iterator = safe_iter<const_iterator_t<T>>;

  private:
    moveable_ptr<T> base = nullptr;

  public:
    iterator begin() { return { std::begin(*base), std::end(*base) }; }
    iterator end() { return { std::end(*base), std::end(*base) }; }
    const_iterator begin() const { return { std::cbegin(*base), std::cend(*base) }; }
    const_iterator end() const { return { std::cend(*base), std::cend(*base) }; }
    const_iterator cbegin() const { return { std::cbegin(*base), std::cend(*base) }; }
    const_iterator cend() const { return { std::cend(*base), std::cend(*base) }; }

  public:
    inline safe(T& t) : base{&t} {}
  };

  template<typename T>
  class safe<T, typename std::enable_if<std::is_const_v<T>>::type> {
  public:
    using const_iterator = safe_iter<const_iterator_t<T>>;

  private:
    moveable_ptr<T> base = nullptr;

  public:
    const_iterator begin() const { return { std::cbegin(*base), std::cend(*base) }; }
    const_iterator end() const { return { std::cend(*base), std::cend(*base) }; }
    const_iterator cbegin() const { return { std::cbegin(*base), std::cend(*base) }; }
    const_iterator cend() const { return { std::cend(*base), std::cend(*base) }; }

  public:
    inline safe(T& t) : base{&t} {}
  };

  template<typename BaseT>
  inline safe_iter<iterator_t<BaseT>> safe_begin(BaseT& b) {
    return { std::begin(b), std::end(b) };
  }
  template<typename BaseT>
  inline safe_iter<typename BaseT::iterator> safe_end(BaseT& b) {
    return { std::begin(b), std::end(b) };
  }

  template<typename BaseT>
  inline safe_iter<iterator_t<BaseT>> safe_cbegin(const BaseT& b) {
    return { std::cbegin(b), std::cend(b) };
  }
  template<typename BaseT>
  inline safe_iter<typename BaseT::iterator> safe_cend(const BaseT& b) {
    return { std::cbegin(b), std::cend(b) };
  }
}

