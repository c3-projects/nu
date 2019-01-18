#include "c3/nu/data.hpp"

#include <string>
#include <cstring>

#include "endian.h"

#define C3_NU_SERIALISE(TYPE, VAR) \
  template<> \
  data serialise(TYPE const& VAR)

#define C3_NU_STATIC_SERIALISE(TYPE, VAR, DATA) \
  template<> \
  void serialise_static(TYPE const& VAR, data_ref DATA)

#define C3_NU_DESERIALISE(TYPE, VAR) \
  template<> \
  TYPE deserialise(data_const_ref VAR)

#define C3_NU_SERIALISEABLE_INT(BITS) \
  C3_NU_STATIC_SERIALISE(int##BITS##_t, i, b) { \
    auto i_be = htobe##BITS (static_cast<uint##BITS##_t>(i)); \
    const uint8_t* i_ptr = reinterpret_cast<const uint8_t*>(&i_be); \
    std::copy(i_ptr, i_ptr + sizeof(i), b.begin()); \
  } \
  C3_NU_DESERIALISE(int##BITS##_t, b) { \
    if (b.size() != (BITS / 8)) \
      throw serialisation_failure("Invalid length"); \
    const auto* i_ptr = reinterpret_cast<const uint##BITS##_t*>(b.data()); \
    return static_cast<int##BITS##_t>(be##BITS##toh (*i_ptr)); \
  } \
  C3_NU_STATIC_SERIALISE(uint##BITS##_t, i, b) { \
    auto i_be = htobe##BITS (i); \
    const uint8_t* i_ptr = reinterpret_cast<const uint8_t*>(&i_be); \
    std::copy(i_ptr, i_ptr + sizeof(i), b.begin()); \
  } \
  C3_NU_DESERIALISE(uint##BITS##_t, b) { \
    if (b.size() != (BITS / 8)) \
      throw serialisation_failure("Invalid length"); \
    const auto* i_ptr = reinterpret_cast<const uint##BITS##_t*>(b.data()); \
    return be##BITS##toh (*i_ptr); \
  }

namespace c3::nu {
  C3_NU_SERIALISE(std::string, str) {
    return { str.begin(), str.end() };
  }
  C3_NU_DESERIALISE(std::string, data) {
    return { data.begin(), data.end() };
  }

  C3_NU_STATIC_SERIALISE(uint8_t, i, b) {
    b[0] = i;
  }
  C3_NU_DESERIALISE(uint8_t, b) {
    if (b.size() != 1)
      throw serialisation_failure("Invalid length");
    else return b[0];
  }

  C3_NU_STATIC_SERIALISE(int8_t, i, b) {
    b[0] = static_cast<uint8_t>(i);
  }
  C3_NU_DESERIALISE(int8_t, b) {
    if (b.size() != 1)
      throw serialisation_failure("Invalid length");
    else return static_cast<int8_t>(b[0]);
  }

  C3_NU_SERIALISEABLE_INT(16);
  C3_NU_SERIALISEABLE_INT(32);
  C3_NU_SERIALISEABLE_INT(64);
}
