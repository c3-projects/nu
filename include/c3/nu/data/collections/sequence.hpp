#pragma once

#include "c3/nu/data.hpp"
#include <c3/nu/int_maths.hpp>

namespace c3::nu {
  template<typename Iter>
  inline data squash_seq(Iter begin, Iter end) {
    using T = typename std::iterator_traits<Iter>::value_type;

    typename std::enable_if<is_static_serialisable_v<T>, data>::type ret;

    for (Iter iter = begin; iter != end; ++iter) {
      auto& i = *iter;

      auto pos = ret.size();
      ret.resize(pos + serialised_size<T>());
      serialise_static<T>(i, { ret.data() + pos, serialised_size<T>() });
    };

    return ret;
  }

  template<typename SizeType, typename Iter>
  inline data squash_seq(Iter begin, Iter end) {
    using T = typename std::iterator_traits<Iter>::value_type;

    typename std::enable_if<!is_static_serialisable_v<T>, data>::type ret;

    for (Iter iter = begin; iter != end; ++iter) {
      auto& i = *iter;

      data buf = serialise(i);

      {
        SizeType len_s = buf.size();
        auto pos = ret.size();
        ret.resize(pos + serialised_size<SizeType>());
        serialise_static<SizeType>(len_s, { ret.data() + pos, serialised_size<SizeType>() });
      }

      ret.insert(ret.end(), buf.begin(), buf.end());
    }

    return ret;
  }

  template<typename T>
  inline std::vector<T> expand_seq(nu::data_const_ref b) {
    std::vector<typename std::enable_if<is_static_serialisable_v<T>, T>::type> ret;

    if (b.size() % serialised_size<T>() != 0)
      throw serialisation_failure("Spare bits in serialised seq");

    for (size_t i = 0; i < static_cast<size_t>(b.size()); i += serialised_size<T>())
      ret.emplace_back(deserialise<T>(b.subspan(i, serialised_size<T>())));

    return ret;
  }

  template<typename T, typename SizeType,
           typename = typename std::enable_if<!is_static_serialisable_v<T>>::type>
  inline std::vector<T> expand_seq(nu::data_const_ref b) {
    std::vector<T> ret;

    while (b.size() > 0) {
      SizeType len_s = deserialise<SizeType>(b.subspan(0, serialised_size<SizeType>()));
      if (!can_cast<size_t>(len_s))
        throw serialisation_failure("Element size overflows size_t");
      size_t len = len_s;
      ret.emplace_back(deserialise<T>(b.subspan(serialised_size<SizeType>(), len)));
      b = b.subspan(serialised_size<SizeType>() + len);
    }

    return ret;
  }
}
