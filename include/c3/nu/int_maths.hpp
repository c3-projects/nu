#pragma once

namespace c3::nu {
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
