#pragma once

#include "c3/nu/data.hpp"

namespace c3::nu {
  inline void squash_static_unsafe(data_ref) {}

  template<typename Head, typename... Tail>
  inline void squash_static_unsafe(data_ref b, Head&& head, Tail&&... tail) {
    auto len = nu::serialised_size<typename remove_all<Head>::type>();
    serialise_static(head, {b.data(), static_cast<data_ref::size_type>(len)});
    if constexpr (sizeof...(Tail) > 0)
      squash_static_unsafe({b.data() + len, static_cast<data_ref::size_type>(b.size() - len)},
                           std::forward<Tail&&>(tail)...);
  }

  template<typename... Input>
  inline data squash_static(Input&&... in) {
    data ret(total_serialised_size<Input...>());
    squash_static_unsafe(ret, in...);
    return ret;
  }

  template<typename Head, typename... Tail>
  inline void expand_static_unsafe(data_const_ref b, Head& head, Tail&... tail) {
    head = deserialise<Head>(b);
    if constexpr (sizeof...(Tail) > 0) {
      auto len = serialised_size<Head>();
      expand_static_unsafe({b.data() + len, static_cast<data_ref::size_type>(b.size() - len)},
                           tail...);
    }
  }

  template<typename... Output>
  inline void expand_static(data_const_ref b, Output&... output) {
    if (total_serialised_size<Output...>() != static_cast<size_t>(b.size()))
      throw std::range_error("Expanding would overrun buffer");
    expand_static_unsafe(b, output...);
  }
}
