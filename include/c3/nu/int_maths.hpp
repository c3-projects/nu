#pragma once

#include <limits>

namespace c3::nu {
  template<typename Lval, typename Rval>
  inline bool try_add(Lval& lv, Rval&& rv) {
    if constexpr (std::numeric_limits<Rval>::digits > std::numeric_limits<Lval>::digits) {
      if (static_cast<Rval>(lv) > static_cast<Rval>(std::numeric_limits<Lval>::max()) - rv)
        return false;
    }
    else if constexpr (std::numeric_limits<Rval>::digits < std::numeric_limits<Lval>::digits) {
      if (lv > std::numeric_limits<Lval>::max() - static_cast<Lval>(rv))
        return false;
    }
    else {
      if (lv > std::numeric_limits<Lval>::max() - rv)
        return false;
    }

    lv += rv;

    return true;
  }

  template<typename IntType>
  constexpr IntType divide_ceil(IntType dividend, IntType devisor) {
    return (dividend + devisor - 1) / devisor;
  }

  template<typename Rep, typename Ret = Rep>
  constexpr Ret constexpr_log(Rep value) {
    Ret ret = 0;

    for (Rep i = value; i != 0; i <<= 1) ++ret;

    return ret;
  }
}
