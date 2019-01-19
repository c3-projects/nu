#pragma once

#include <cstdint>
#include <cstdlib>

#include "c3/nu/int_maths.hpp"

namespace c3::nu {
  template<bool> struct range;

  template<size_t Bits, typename = range<true>>
  class integral;

  template<size_t Bits, typename = range<true>>
  class integral_fast;

  template<size_t Bits>
  using integral_t = typename integral<Bits>::type;

  template<size_t Bits>
  using integral_fast_t = typename integral_fast<Bits>::type;

  template<uint64_t MaxValue>
  using integral_upto_t = integral_t<constexpr_log(MaxValue)>;

  template<uint64_t MaxValue>
  using integral_fast_upto_t = integral_fast_t<constexpr_log(MaxValue)>;

#define C3_NU_INTEGRAL_TYPE(TYPE, MIN, MAX) \
  template<size_t Bits> \
  class integral<Bits, range<(Bits > MIN && Bits <= MAX)>> { \
    public: using type = TYPE; \
  };
#define C3_NU_INTEGRAL_FAST_TYPE(TYPE, MIN, MAX) \
  template<size_t Bits> \
  class integral_fast<Bits, range<(Bits > MIN && Bits <= MAX)>> { \
    public: using type = TYPE; \
  };

  C3_NU_INTEGRAL_TYPE(uint8_t,   0,  8)
  C3_NU_INTEGRAL_TYPE(uint16_t,  8, 16)
  C3_NU_INTEGRAL_TYPE(uint32_t, 16, 32)
  C3_NU_INTEGRAL_TYPE(uint64_t, 32, 64)

  C3_NU_INTEGRAL_FAST_TYPE(uint_fast8_t,   0,  8)
  C3_NU_INTEGRAL_FAST_TYPE(uint_fast16_t,  8, 16)
  C3_NU_INTEGRAL_FAST_TYPE(uint_fast32_t, 16, 32)
  C3_NU_INTEGRAL_FAST_TYPE(uint_fast64_t, 32, 64)

#undef C3_NU_INTEGRAL_TYPE
#undef C3_NU_INTEGRAL_FAST_TYPE
}
