#pragma once

// WTF SORT OF IDIOT CAME UP WITH TERMINATE ON BOUNDS CHECK??
#define GSL_THROW_ON_CONTRACT_VIOLATION

#include <cstdint>
#include <vector>
#include <exception>
#include <array>
#include <type_traits>

#include <string>
#include <cstring>

#include <gsl/span>

static_assert(sizeof(size_t) >= sizeof(uint32_t), "Cannot safely serialise");

#include "c3/nu/data/helpers.hpp"
#include "c3/nu/endian.hpp"

namespace c3::nu {
  // Making this a char allows implicit upcasting
  constexpr uint8_t dynamic_size = 0;

  template<class T, class U=
    typename std::remove_cv<
    typename std::remove_pointer<
    typename std::remove_reference<
    typename std::remove_extent<
    T
    >::type
    >::type
    >::type
    >::type
    > struct remove_all : remove_all<U> {};
  template<class T> struct remove_all<T, T> { typedef T type; };

  template<typename Test>
  struct is_fixed_span : std::false_type {};

  template<typename T, std::ptrdiff_t Len>
  struct is_fixed_span<gsl::span<T, Len>> : std::true_type {};
  template<typename T, std::ptrdiff_t Len>
  struct is_fixed_span<gsl::span<const T, Len>> : std::true_type {};
  template<typename T, std::ptrdiff_t Len>
  struct is_fixed_span<gsl::span<volatile T, Len>> : std::true_type {};
  template<typename T, std::ptrdiff_t Len>
  struct is_fixed_span<gsl::span<const volatile T, Len>> : std::true_type {};

  template<typename T>
  constexpr bool is_fixed_span_v = is_fixed_span<T>::value;

  template<typename Test>
  struct is_array : std::false_type {};

  template<typename T, size_t Len>
  struct is_array<std::array<T, Len>> : std::true_type {};
  template<typename T, size_t Len>
  struct is_array<std::array<const T, Len>> : std::true_type {};
  template<typename T, size_t Len>
  struct is_array<std::array<volatile T, Len>> : std::true_type {};
  template<typename T, size_t Len>
  struct is_array<std::array<const volatile T, Len>> : std::true_type {};

  template<typename T>
  constexpr bool is_array_v = is_array<T>::value;

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

  template<typename T>
  inline T deserialise(data_const_ref d) {
    if constexpr (std::is_base_of_v<serialisable<T>, T>)
      return T::_deserialise(d);
    else
      return static_cast<T>(deserialise<typename std::underlying_type<T>::type>(d));
  }

  template<typename T>
  inline T deserialise(const uint8_t* d) {
    return deserialise({d, serialised_size<T>()});
  }

  template<typename T>
  class static_serialisable;

  template<typename T>
  constexpr size_t serialised_size() {
    if constexpr (std::is_base_of_v<static_serialisable<T>, T>)
      return T::_serialised_size;
    else if constexpr (is_fixed_span_v<T>)
      return serialised_size<typename T::value_type>() * T::extent;
    else if constexpr (is_array_v<T>)
      return serialised_size<typename T::value_type>() * std::tuple_size<T>::value;
    else //if constexpr (std::is_enum_v<T>)
      return serialised_size<typename std::underlying_type<T>::type>();
  }

  template<typename A, typename... StaticT>
  constexpr size_t total_serialised_size() {
    if constexpr (sizeof...(StaticT) == 0)
      return serialised_size<typename remove_all<A>::type>();
    else
      return serialised_size<typename remove_all<A>::type>() + total_serialised_size<StaticT...>();
  }

  template<typename A, typename... StaticT>
  using static_buffer = std::array<uint8_t, total_serialised_size<A, StaticT...>()>;

  /// XXX: Does not check size of buffer!!!
  template<typename T>
  void serialise_static(const T& t, data_ref d) {
    if constexpr (std::is_base_of_v<static_serialisable<T>, T>)
      t._serialise_static(d);
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

  std::string base64_encode_data(data_const_ref);
  data base64_decode_data(const std::string&);

  template<typename T>
  inline std::string base64_encode(const T& t) {
    return base64_encode_data(serialise(t));
  }

  template<typename T>
  inline T base64_decode(const std::string& str) {
    return deserialise<T>(base64_decode_data(str));
  }

  /// XXX: does not check the size of the output buffer
  template<typename Head, typename... Tail>
  inline void serialise_all(gsl::span<data> output, Head head, Tail... tail){
    output[0] = serialise(head);
    if constexpr (sizeof...(Tail) > 0)
      serialise_all(output.subspan(1), tail...);
  }

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
}

#include "c3/nu/data/clean_helpers.hpp"
