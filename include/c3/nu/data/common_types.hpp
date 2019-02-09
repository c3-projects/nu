#pragma once

#include "c3/nu/data/base.hpp"

namespace c3::nu {
#define C3_NU_SERIALISEABLE_INT(BITS) \
  template<> \
  inline void serialise_static<int##BITS##_t>(const int##BITS##_t& i, data_ref b) { \
    auto i_be = htobe##BITS (static_cast<uint##BITS##_t>(i)); \
    const uint8_t* i_ptr = reinterpret_cast<const uint8_t*>(&i_be); \
    std::copy(i_ptr, i_ptr + sizeof(i), b.begin()); \
  } \
  template<> \
  inline int##BITS##_t deserialise<int##BITS##_t>(data_const_ref b) { \
    if (b.size() != (BITS / 8)) \
      throw serialisation_failure("Invalid length"); \
    const auto* i_ptr = reinterpret_cast<const uint##BITS##_t*>(b.data()); \
    return static_cast<int##BITS##_t>(be##BITS##toh (*i_ptr)); \
  } \
  template<> \
  inline void serialise_static<uint##BITS##_t>(const uint##BITS##_t& i, data_ref b) { \
    auto i_be = htobe##BITS (i); \
    const uint8_t* i_ptr = reinterpret_cast<const uint8_t*>(&i_be); \
    std::copy(i_ptr, i_ptr + sizeof(i), b.begin()); \
  } \
  template<> \
  inline uint##BITS##_t deserialise<uint##BITS##_t>(data_const_ref b) { \
    if (b.size() != (BITS / 8)) \
      throw serialisation_failure("Invalid length"); \
    const auto* i_ptr = reinterpret_cast<const uint##BITS##_t*>(b.data()); \
    return be##BITS##toh (*i_ptr); \
  } \
  template<> \
  constexpr size_t serialised_size<int##BITS##_t>() { return BITS / 8; } \
  template<> \
  constexpr size_t serialised_size<uint##BITS##_t>() { return BITS / 8; } \

  C3_NU_SERIALISEABLE_INT(16);
  C3_NU_SERIALISEABLE_INT(32);
  C3_NU_SERIALISEABLE_INT(64);

#undef C3_NU_SERIALISEABLE_INT

  // These are needed in this header file
  template<>
  inline data serialise(const std::string& str) {
    return { str.begin(), str.end() };
  }
  template<>
  inline std::string deserialise(data_const_ref b) {
    return { b.begin(), b.end() };
  }

  template<>
  constexpr size_t serialised_size<uint8_t>() { return 1; }
  template<>
  inline void serialise_static(const uint8_t& i, data_ref b) {
    b[0] = i;
  }
  template<>
  inline uint8_t deserialise(data_const_ref b){
    if (b.size() != 1)
      throw serialisation_failure("Invalid length");
    else return b[0];
  }

  template<>
  constexpr size_t serialised_size<int8_t>() { return 1; }
  template<>
  inline void serialise_static(const int8_t& i, data_ref b) {
    b[0] = static_cast<uint8_t>(i);
  }
  template<>
  inline int8_t deserialise(data_const_ref b){
    if (b.size() != 1)
      throw serialisation_failure("Invalid length");
    else return static_cast<int8_t>(b[0]);
  }

  template<>
  constexpr size_t serialised_size<char>() { return 1; }
  template<>
  inline void serialise_static(const char& i, data_ref b) {
    b[0] = static_cast<uint8_t>(i);
  }
  template<>
  inline char deserialise(data_const_ref b){
    if (b.size() != 1)
      throw serialisation_failure("Invalid length");
    else return static_cast<int8_t>(b[0]);
  }

  template<>
  inline data serialise(const data& b) { return b; }
  template<>
  inline data deserialise(data_const_ref b) { return data(b.begin(), b.end()); }

  template<>
  inline data serialise(const data_const_ref& b) { return data(b.begin(), b.end()); }
  template<>
  inline data_const_ref deserialise(data_const_ref b) { return b; }

  inline data serialise(const char* cstr) {
    return { cstr, cstr + ::strlen(cstr) };
  }

  template<typename T, size_t Len>
  inline void serialise_static(std::array<T, Len> input, data_ref b) {
    for (auto i : input) {
      serialise_static(b.subspan(0, serialised_size<T>()));
      b = b.subspan(serialised_size<T>());
    }
  }

  template<typename T, size_t Len>
  inline std::array<T, Len> deserialise(data_const_ref b) {
    if (static_cast<size_t>(b.size()) != serialised_size<std::array<T, Len>>())
      throw serialisation_failure("Buffer size is different from expected value");

    std::array<T, Len> ret;
    for (auto& i : ret) {
      i = deserialise<T>(b.subspan(0, serialised_size<T>()));
      b = b.subspan(serialised_size<T>());
    }
    return ret;
  }
}
