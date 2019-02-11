#pragma once

#include "c3/nu/int_maths.hpp"
#include "c3/nu/data.hpp"

namespace c3::nu {
  using default_size_type = uint32_t;
  /// Used to enforce hybrid/static serialisation
  using hybrid_struct = void;

  template<typename SizeType = hybrid_struct, typename Head, typename... Tail>
  inline void _squash_internal(data& acc, Head&& head, Tail&&... tail) {
    if constexpr (is_static_serialisable_v<Head>) {
      auto current_len = acc.size();
      acc.resize(current_len + serialised_size<Head>());
      data_ref to_fill{acc.data() + current_len, serialised_size<Head>()};
      serialise_static(head, to_fill);
    }
    else {
      data buf = serialise(head);
      if constexpr (sizeof...(Tail) != 0) {
        if (buf.size() > std::numeric_limits<SizeType>::max()) {
          throw serialisation_failure("SizeType was too small to hold a value");
        }

        SizeType len = static_cast<SizeType>(buf.size());
        auto current_len = acc.size();
        acc.resize(current_len + serialised_size<SizeType>());
        data_ref to_fill{acc.data() + current_len, serialised_size<SizeType>()};
        serialise_static(len, to_fill);
      }
      acc.insert(acc.end(), buf.begin(), buf.end());
    }

    if constexpr (sizeof...(Tail) != 0)
      _squash_internal<SizeType>(acc, tail...);
  }

  template<typename SizeType = hybrid_struct, typename Head, typename... Tail>
  inline data squash(Head&& head, Tail... tail) {
    data ret;
    _squash_internal<SizeType>(ret, head, tail...);
    return ret;
  }

  template<typename SizeType = hybrid_struct, typename Head, typename... Tail>
  inline void expand(data_const_ref b, Head& head, Tail&... tail) {
    // We do use this, but only sometimes
    size_t our_chunk_size = 0;
    // This silences the warning on the final case, where we don't actually use it
    (void) our_chunk_size;

    if constexpr (is_static_serialisable_v<Head>) {
      our_chunk_size = serialised_size<Head>();
      // Whilst a subspan is not strictly necessary, it does a bounds check, so we don't have to
      head = deserialise<Head>(b.subspan(0, serialised_size<Head>()));
    }
    else {
      if (!try_add(our_chunk_size, serialised_size<SizeType>()))
        throw serialisation_failure("Element size overflows size_t");

      if constexpr (sizeof...(Tail) != 0) {
        SizeType len_s = deserialise<SizeType>(b.subspan(0, serialised_size<SizeType>()));
        if (!try_add(our_chunk_size, len_s))
          throw serialisation_failure("Possibly fake element size overflows size_t");
         head = deserialise<Head>(b.subspan(serialised_size<SizeType>(), static_cast<size_t>(len_s)));
      }
      else
        head = deserialise<Head>(b);
    }

    // Subspan does a bounds check, so we don't have to
    if constexpr (sizeof...(Tail) != 0)
      expand<SizeType>(b.subspan(our_chunk_size), tail...);
  }
}
