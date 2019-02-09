//! A bunch of helper macros for implementing serialisation

// We don't want this, as it does make sense for this to included multiple times
// #pragma once

#include <type_traits>

#define C3_NU_DEFER_SERIALISATION_TYPE(TYPE, BASE_TYPE) \
  template<typename T> \
  friend T c3::nu::deserialise(c3::nu::data_const_ref); \
  private: \
  static TYPE _deserialise(c3::nu::data_const_ref b) { \
    return static_cast<TYPE>(c3::nu::deserialise<BASE_TYPE>(b)); \
  } \
  c3::nu::data _serialise() const override { \
    return c3::nu::serialise(static_cast<const BASE_TYPE&>(*this)); \
  }

#define C3_NU_DEFER_STATIC_SERIALISATION_TYPE(TYPE, BASE_TYPE) \
  template<typename T> \
  friend T c3::nu::deserialise(c3::nu::data_const_ref); \
  template<typename T> \
  friend constexpr size_t c3::nu::serialised_size(); \
  private: \
  static constexpr size_t _serialised_size = c3::nu::serialised_size<BASE_TYPE>(); \
  private: \
  static TYPE _deserialise(c3::nu::data_const_ref b) { \
    return static_cast<TYPE>(c3::nu::deserialise<BASE_TYPE>(b)); \
  } \
  void _serialise_static(c3::nu::data_ref b) const override { \
    c3::nu::serialise_static(static_cast<const BASE_TYPE&>(*this), b); \
  }

#define C3_NU_DEFER_SERIALISATION_VAR(TYPE, BASE_VAR) \
  template<typename T> \
  friend T c3::nu::deserialise(c3::nu::data_const_ref); \
  private: \
  static TYPE _deserialise(c3::nu::data_const_ref b) { \
    return static_cast<TYPE>(c3::nu::deserialise<decltype(BASE_VAR)>(b)); \
  } \
  c3::nu::data _serialise() const override { \
    return c3::nu::serialise(static_cast<const decltype(BASE_VAR)&>(BASE_VAR)); \
  }

#define C3_NU_DEFER_STATIC_SERIALISATION_VAR(TYPE, BASE_VAR) \
  template<typename T> \
  friend T c3::nu::deserialise(c3::nu::data_const_ref); \
  template<typename T> \
  friend constexpr size_t c3::nu::serialised_size(); \
  private: \
  static constexpr size_t _serialised_size = c3::nu::serialised_size<decltype(BASE_VAR)>(); \
  private: \
  static TYPE _deserialise(c3::nu::data_const_ref b) { \
    return static_cast<TYPE>(c3::nu::deserialise<decltype(BASE_VAR)>(b)); \
  } \
  void _serialise_static(c3::nu::data_ref b) const override { \
    c3::nu::serialise_static(static_cast<const decltype(BASE_VAR)&>(BASE_VAR), b); \
  }

#define C3_NU_DEFINE_DESERIALISE(TYPE, DATA_VAR_NAME) \
  template<typename T> \
  friend T c3::nu::deserialise(c3::nu::data_const_ref); \
  private: \
  static TYPE _deserialise(c3::nu::data_const_ref DATA_VAR_NAME)

#define C3_NU_DEFINE_STATIC_DESERIALISE(TYPE, SERIALISED_SIZE, DATA_VAR_NAME) \
  template<typename T> \
  friend T c3::nu::deserialise(c3::nu::data_const_ref); \
  template<typename T> \
  friend constexpr size_t c3::nu::serialised_size(); \
  private: \
  static constexpr size_t _serialised_size = SERIALISED_SIZE; \
  static TYPE _deserialise(c3::nu::data_const_ref DATA_VAR_NAME)

#define C3_NU_SERIALISED_SIZE(TYPE, VALUE) \
  template<> \
  constexpr size_t c3::nu::serialised_size<TYPE>() { return VALUE; }
#define C3_NU_SERIALISE_STATIC_WRAPPER(TYPE) \
  template<> \
  inline data c3::nu::serialise<TYPE>(const TYPE& t) { \
    data ret(serialised_size<TYPE>()); \
    serialise_static<TYPE>(t, ret);\
    return ret; \
  }
