#pragma once

#include <cstdint>
#include <vector>
#include <exception>
#include <array>
#include <type_traits>

#include <string>
#include <cstring>

#include "c3/nu/data/span_deps.hpp"
#include "c3/nu/data/helpers.hpp"
#include "c3/nu/endian.hpp"
#include "c3/nu/sfinae.hpp"

namespace c3::nu {
  // Making this a char allows implicit upcasting
  constexpr uint8_t dynamic_size = 0;

  using data = std::vector<uint8_t>;
  using data_ref = gsl::span<uint8_t>;
  using data_const_ref = gsl::span<const uint8_t>;
  template<size_t Len>
  using static_data = std::array<uint8_t, Len>;

  class serialisation_failure : public std::exception {
  public:
    const char* msg;

  public:
    const char* what() const noexcept override { return msg; }

  public:
    serialisation_failure(decltype(msg) msg = "Serialisation failed") : msg{msg} {}
  };

  /// Calculates the floor of the base 2^bits logarithm of i
  template<typename IntType>
  constexpr uint8_t log_int_bitwise(IntType i, size_t bits) {
    static_assert(sizeof(i) < std::numeric_limits<IntType>::max(), "log of int could cause overflow");
    uint8_t count = 0;

    // 1v1 me
    for (; i != 0; i >>= bits, ++count);
    return count;
  }

  template<typename T>
  uint64_t total_buffers_size(T buffers) {
    uint64_t ret = 0;
    for (auto i : buffers)
      ret += static_cast<uint64_t>(i.size());
    return ret;
  }

  template<typename T>
  class serialisable;

  template<typename T>
  void serialise_static(const T& t, data_ref d);

  template<typename T>
  constexpr size_t serialised_size();

  /// XXX: does not strip qualifiers, as that would confuse parameter type
  template<typename T>
  data serialise(const T& t) {
    if constexpr (std::is_base_of_v<serialisable<T>, T>)
      return static_cast<const serialisable<T>&>(t)._serialise();
    else {
      data ret(serialised_size<T>());
      serialise_static<T>(t, ret);
      return ret;
    }
  }

  inline data serialise(data&& b) { return std::forward<data&&>(b); }

  /// XXX: does not strip qualifiers, as that would confuse return type
  template<typename T>
  inline T deserialise(data_const_ref b) {
    if constexpr (std::is_base_of_v<serialisable<T>, T>)
      return T::_deserialise(b);
    else if constexpr (is_static_serialisable_array_v<T>) {
      T ret;
      std::copy(b.begin(), b.end(), ret.begin());
      return ret;
    }
    else
      return static_cast<T>(deserialise<typename std::underlying_type<T>::type>(b));
  }

  template<typename T>
  inline T deserialise(const uint8_t* d) {
    return deserialise({d, serialised_size<T>()});
  }

  template<typename T>
  class static_serialisable;

  template<typename T, typename = void>
  struct is_static_serialisable : std::false_type {};

  template<typename T>
  struct is_static_serialisable<T,
      typename std::enable_if<
        static_cast<bool>(serialised_size<T>())
      >::type> : std::true_type {};
  template<typename T>
  constexpr bool is_static_serialisable_v = is_static_serialisable<T>::value;

  template<typename T, typename = void>
  struct is_static_serialisable_array : std::false_type {};

  template<typename T>
  struct is_static_serialisable_array<T,
      typename std::enable_if<
        is_array_v<T> &&
        is_static_serialisable_v<typename T::value_type>
      >::type> : std::true_type {};
  template<typename T>
  constexpr bool is_static_serialisable_array_v = is_static_serialisable<T>::value;

  /// XXX: returns 0 if not statically serialisable
  template<typename T>
  constexpr size_t serialised_size() {
    if constexpr (!std::is_same_v<typename remove_all<T>::type, T>)
      return serialised_size<typename remove_all<T>::type>();
    else if constexpr (std::is_base_of_v<static_serialisable<T>, T>)
      return T::_serialised_size;
    else if constexpr (is_fixed_span_v<T>)
      return serialised_size<typename T::value_type>() * T::extent;
    else if constexpr (is_static_serialisable_array_v<T>)
      return serialised_size<typename T::value_type>() * std::tuple_size<T>::value;
    else if constexpr (std::is_enum_v<T>)
      return serialised_size<typename std::underlying_type<T>::type>();
    else
      return 0;
  }

  template<typename A, typename... StaticT>
  constexpr size_t total_serialised_size() {
    if constexpr (sizeof...(StaticT) == 0)
      return serialised_size<A>();
    else
      return serialised_size<A>() + total_serialised_size<StaticT...>();
  }

  template<typename A, typename... StaticT>
  using static_buffer = std::array<uint8_t, total_serialised_size<A, StaticT...>()>;

  /// XXX: Does not check size of buffer!!!
  template<typename T>
  void serialise_static(const T& t, data_ref d) {
    if constexpr (std::is_base_of_v<static_serialisable<T>, T>)
      t._serialise_static(d);
    else if constexpr (is_static_serialisable_array_v<T>)
      std::copy(t.begin(), t.end(), d.begin());
    else
      serialise_static(static_cast<typename std::underlying_type<T>::type>(t), d);
  }

  template<typename T>
  class serialisable {
    friend data serialise<T>(const T&);
  private:
    virtual data _serialise() const = 0;

  public:
    virtual ~serialisable() = default;
  };

  template<typename T>
  class static_serialisable : public serialisable<T> {
    friend data serialise<T>(const T&);
    friend void serialise_static<T>(const T&, data_ref);
  private:
    virtual void _serialise_static(data_ref data) const = 0;

  private:
    data _serialise() const override {
      data ret(serialised_size<T>());
      _serialise_static(ret);
      return ret;
    }

  public:
    virtual ~static_serialisable() = default;
  };

  /// XXX: does not check the size of the output buffer
  template<typename Head, typename... Tail>
  inline void serialise_all(gsl::span<data> output, Head head, Tail... tail){
    output[0] = serialise(head);
    if constexpr (sizeof...(Tail) > 0)
      serialise_all(output.subspan(1), tail...);
  }
}

#include "c3/nu/data/clean_helpers.hpp"
