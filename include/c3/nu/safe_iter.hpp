#pragma once

#include <iterator>

namespace c3::nu {
  template<typename Iter, typename IterType = typename std::iterator_traits<Iter>::iterator_category>
  class safe_iter;

  template<typename Iter>
  class safe_iter_base {
    template<typename Iter, typename IterType>
    friend class safe_iter;

  private:
    Iter base;
    Iter end;

  public:
    inline void operator++() {
      if (base == end) throw std::out_of_range("Tried to iterate past end");
      ++base;
      return *this;
    }
    inline decltype(auto) operator*() { return *base; }

  public:
    inline bool operator==(const safe_iter_base& other) const { return base == other.base; }
    inline bool operator!=(const safe_iter_base& other) const { return base != other.base; }
  };

  template<typename Iter>
  class safe_iter<Iter, std::input_iterator_tag> : public safe_iter_base<Iter> {
  public:
    using safe_iter_base<Iter>::operator++;
  public:
    using safe_iter_base<Iter>::operator==;
    using safe_iter_base<Iter>::operator!=;

  public:
    safe_iter(Iter base, Iter end) : safe_iter_base<Iter>(base, end) {}
  };
  template<typename Iter>
  class safe_iter<Iter, std::output_iterator_tag> : public safe_iter_base<Iter> {
  public:
    using safe_iter_base<Iter>::operator++;
  public:
    using safe_iter_base<Iter>::operator==;
    using safe_iter_base<Iter>::operator!=;

  public:
    safe_iter(Iter base, Iter end) : safe_iter_base<Iter>(base, end) {}
  };

  template<typename Iter>
  class safe_iter<Iter, std::forward_iterator_tag> {
  public:
    using safe_iter_base<Iter>::operator++;
  public:
    using safe_iter_base<Iter>::operator==;
    using safe_iter_base<Iter>::operator!=;

  public:
    safe_iter(Iter base, Iter end) : safe_iter_base<Iter>(base, end) {}
    safe_iter() {}
  };

  template<typename Iter>
  class safe_iter<Iter, std::bidirectional_iterator_tag> :
      public safe_iter<Iter, std::forward_iterator_tag> {
  public:
    using safe_iter_base<Iter>::operator==;
    using safe_iter_base<Iter>::operator!=;
  };

  template<typename Iter>
  class safe_iter<Iter, std::random_access_iterator_tag> :
      public safe_iter<Iter, std::bidirectional_iterator_tag> {
  public:
    using safe_iter_base<Iter>::operator==;
    using safe_iter_base<Iter>::operator!=;
  };
}
