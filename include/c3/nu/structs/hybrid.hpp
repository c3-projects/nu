#pragma once

#include "c3/nu/data.hpp"

namespace c3::nu {
  template<typename Head, typename... Tail>
  inline size_t _get_hybrid_prealloc() {
    if constexpr (sizeof...(Tail) == 0)
      return 0;
    else {
      return serialised_size<typename remove_all<Head>::type>() + _get_hybrid_prealloc<Tail...>();
    }
  }

  template<typename Head, typename... Tail>
  inline void _squash_hybrid_internal(data& b, size_t offset, Head&& head, Tail... tail) {
    if constexpr (sizeof...(Tail) == 0) {
      data final = serialise(head);
      b.insert(b.end(), final.begin(), final.end());
    }
    else {
      size_t len = serialised_size<typename remove_all<Head>::type>();
      serialise_static(head, {b.data() + offset, static_cast<ssize_t>(len)});
      _squash_hybrid_internal(b, offset + len, tail...);
    }
  }

  template<typename... Input>
  inline data squash_hybrid(Input&&... input) {
    data ret(_get_hybrid_prealloc<Input...>());
    _squash_hybrid_internal(ret, 0, input...);
    return ret;
  }

  template<typename Head, typename... Tail>
  inline void expand_hybrid(data_const_ref b, Head& head, Tail... tail) {
    if constexpr (sizeof...(Tail) == 0) {
      head = deserialise<Head>(b);
    }
    else {
      ssize_t len = static_cast<ssize_t>(serialised_size<Head>());
      head = deserialise<Head>({b.data(), len});
      expand_hybrid({b.data() + len, b.size() - len}, tail...);
    }
  }
}
