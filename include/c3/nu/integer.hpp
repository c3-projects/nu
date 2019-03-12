#pragma once

#include <limits>
#include "c3/nu/sfinae.hpp"
#include <cstdint>
#include <cstdlib>
#include <algorithm>

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

  template<bool> struct range;

  template<size_t Bits, typename = range<true>>
  class integer;

  template<size_t Bits, typename = range<true>>
  class integer_fast;

  template<size_t Bits>
  using integer_t = typename integer<Bits>::type;

  template<size_t Bits>
  using integer_fast_t = typename integer_fast<Bits>::type;

  template<uint64_t MaxValue>
  using integer_upto_t = integer_t<constexpr_log(MaxValue)>;

  template<uint64_t MaxValue>
  using integer_fast_upto_t = integer_fast_t<constexpr_log(MaxValue)>;

  template<size_t Bits, typename = range<true>>
  class integer_signed;

  template<size_t Bits, typename = range<true>>
  class integer_signed_fast;

  template<size_t Bits>
  using integer_signed_t = typename integer_signed<Bits>::type;

  template<size_t Bits>
  using integer_signed_fast_t = typename integer_signed_fast<Bits>::type;

  template<uint64_t MaxValue>
  using integer_signed_upto_t = integer_signed_t<constexpr_log(MaxValue)>;

  template<uint64_t MaxValue>
  using integer_signed_fast_upto_t = integer_signed_fast_t<constexpr_log(MaxValue)>;

  using integer_biggest_t = uint64_t;
  using integer_signed_biggest_t = int64_t;

  template<typename Holder, typename Holdee>
  constexpr bool integer_can_hold() {
    return std::is_signed_v<Holder> == std::is_signed_v<Holdee> &&
        std::numeric_limits<Holder>::digits >= std::numeric_limits<Holdee>::digits;
  }

  template<typename IntA, typename IntB>
  using integer_bigger_t = nth_type<integer_can_hold<IntA, IntB>, IntA, IntB>;

  template<typename Holder, typename Holdee>
  constexpr bool integer_can_hold(Holdee i) {
    if constexpr (std::is_unsigned_v<Holder> && std::is_unsigned_v<Holdee>) {
      constexpr integer_biggest_t max = std::numeric_limits<Holder>::max();
      return static_cast<integer_biggest_t>(i) <= max;
    }
    else if constexpr (std::is_signed_v<Holder> && std::is_signed_v<Holdee>) {
      constexpr integer_signed_biggest_t max = std::numeric_limits<Holder>::max();
      constexpr integer_signed_biggest_t min = std::numeric_limits<Holder>::min();
      return static_cast<integer_signed_biggest_t>(i) <= max &&
          static_cast<integer_signed_biggest_t>(i) >= min;

    }
    else if constexpr (std::is_unsigned_v<Holder> && std::is_signed_v<Holdee>) {
      constexpr integer_biggest_t max = std::numeric_limits<Holder>::max();
      return i >= 0 && static_cast<integer_biggest_t>(i) <= max;
    }
    else { // if constexpr (std::is_signed_v<Holder> && std::is_unsigned_v<Holdee>) {
      constexpr integer_biggest_t max = std::numeric_limits<Holder>::max();
      return static_cast<integer_biggest_t>(i) <= max;
    }
  }

  template<typename Lval, typename Rval>
  constexpr bool integer_try_add(Lval& lv, Rval&& rv) {
    static_assert(std::is_signed_v<Lval> == std::is_signed_v<Rval>,
                  "The mingling of signed and unsigned ints is a path fraught with danger");

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

  template<typename To, typename From>
  constexpr To int_cast(From i) {
    if (!integer_can_hold<To>(i))
      throw std::out_of_range("Cannot fit int in required type");
    return static_cast<To>(i);
  }

#define C3_NU_integer_TYPE(TYPE, MIN, MAX) \
  template<size_t Bits> \
  class integer<Bits, range<(Bits > MIN && Bits <= MAX)>> { \
    public: using type = TYPE; \
  };
#define C3_NU_integer_FAST_TYPE(TYPE, MIN, MAX) \
  template<size_t Bits> \
  class integer_fast<Bits, range<(Bits > MIN && Bits <= MAX)>> { \
    public: using type = TYPE; \
  };

  C3_NU_integer_TYPE(uint8_t,   0,  8)
  C3_NU_integer_TYPE(uint16_t,  8, 16)
  C3_NU_integer_TYPE(uint32_t, 16, 32)
  C3_NU_integer_TYPE(uint64_t, 32, 64)

  C3_NU_integer_FAST_TYPE(uint_fast8_t,   0,  8)
  C3_NU_integer_FAST_TYPE(uint_fast16_t,  8, 16)
  C3_NU_integer_FAST_TYPE(uint_fast32_t, 16, 32)
  C3_NU_integer_FAST_TYPE(uint_fast64_t, 32, 64)

#define C3_NU_integer_SIGNED_TYPE(TYPE, MIN, MAX) \
  template<size_t Bits> \
  class integer_signed<Bits, range<(Bits > MIN && Bits <= MAX)>> { \
    public: using type = TYPE; \
  };
#define C3_NU_integer_SIGNED_FAST_TYPE(TYPE, MIN, MAX) \
  template<size_t Bits> \
  class integer_signed_fast<Bits, range<(Bits > MIN && Bits <= MAX)>> { \
    public: using type = TYPE; \
  };

  C3_NU_integer_SIGNED_TYPE(int8_t,   0,  8)
  C3_NU_integer_SIGNED_TYPE(int16_t,  8, 16)
  C3_NU_integer_SIGNED_TYPE(int32_t, 16, 32)
  C3_NU_integer_SIGNED_TYPE(int64_t, 32, 64)

  C3_NU_integer_SIGNED_FAST_TYPE(int_fast8_t,   0,  8)
  C3_NU_integer_SIGNED_FAST_TYPE(int_fast16_t,  8, 16)
  C3_NU_integer_SIGNED_FAST_TYPE(int_fast32_t, 16, 32)
  C3_NU_integer_SIGNED_FAST_TYPE(int_fast64_t, 32, 64)

#undef C3_NU_integer_TYPE
#undef C3_NU_integer_FAST_TYPE
}
