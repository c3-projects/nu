#pragma once

namespace c3::nu {
  template<typename IntType>
  constexpr IntType divide_ceil(IntType dividend, IntType devisor) {
    return (dividend + devisor - 1) / devisor;
  }

  template<typename Rep, typename Ret, Rep Value>
  constexpr Ret constexpr_log() {
    Ret ret = 0;

    for (Rep i = Value; i != 0; i <<= 1) ++ret;

    return ret;
  }
}
