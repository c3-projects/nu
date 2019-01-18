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

#include "c3/upsilon/data/helpers.hpp"

namespace c3::upsilon {
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

  using data = std::vector<uint8_t>;
  using data_ref = gsl::span<uint8_t>;
  using data_const_ref = gsl::span<const uint8_t>;
  using input_buffers = gsl::span<const data_const_ref>;
  using output_buffers = gsl::span<const data_ref>;
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

  /// Serialises a sequence of dynamic buffers, so that they can be deserialised separately
  data serialise_dynamic_struct(input_buffers);
  /// Deserialises a sequence of dynamic buffers, so that they can be deserialised separately
  std::vector<data_const_ref> deserialise_dynamic_struct(data_const_ref);
  /// Serialises a sequence of static buffers, so that they can be deserialised separately
  data serialise_static_struct(input_buffers);
  /// Deserialises a sequence of static buffers, so that they can be deserialised separately
  data deserialise_static_struct(data_const_ref);

  /// Derialises a sequence of buffers
  ///
  /// Returns a series of spans pointing to the input data
  std::vector<data_ref> deserialise_struct(data_const_ref);

  class input_buffer_ref {
  private:
    std::array<data_const_ref, 1> _arr;

  public:
    inline operator input_buffers() {
      return { _arr };
    }

  public:
    constexpr input_buffer_ref(data_const_ref r) : _arr{r} {}
  };

  class output_buffer_ref {
  private:
    std::array<data_ref, 1> _arr;

  public:
    inline operator output_buffers() {
      return { _arr };
    }

  public:
    constexpr output_buffer_ref(data_ref r) : _arr{r} {}
  };

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
  T deserialise(data_const_ref d) {
    if constexpr (std::is_base_of_v<serialisable<T>, T>)
      return T::_deserialise(d);
    else
      return static_cast<T>(deserialise<typename std::underlying_type<T>::type>(d));
  }

  template<typename T>
  class static_serialisable;

  template<typename T>
  constexpr size_t serialised_size() {
    if constexpr (std::is_base_of_v<static_serialisable<T>, T>)
      return T::_serialised_size;
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

#define C3_UPSILON_IS_SERIALISABLE(TYPE) \
  template<> \
  data serialise<TYPE>(TYPE const&); \
  template<> \
  TYPE deserialise<TYPE>(data_const_ref);

#define C3_UPSILON_IS_STATIC_SERIALISABLE(TYPE, SIZE) \
  template<> \
  void serialise_static<TYPE>(const TYPE&, data_ref); \
  template<> \
  constexpr size_t serialised_size<TYPE>() { return SIZE; } \
  template<> \
  inline data serialise<TYPE>(const TYPE& t) { \
    data ret(serialised_size<TYPE>()); \
    serialise_static<TYPE>(t, ret);\
    return ret; \
  } \
  template<> \
  TYPE deserialise<TYPE>(data_const_ref);

  // These are needed in this header file
  C3_UPSILON_IS_SERIALISABLE(std::string);
  C3_UPSILON_IS_SERIALISABLE(gsl::span<const data_const_ref>);

  C3_UPSILON_IS_STATIC_SERIALISABLE(uint8_t, 8 / 8);
  C3_UPSILON_IS_STATIC_SERIALISABLE(uint16_t, 16 / 8);
  C3_UPSILON_IS_STATIC_SERIALISABLE(uint32_t, 32 / 8);
  C3_UPSILON_IS_STATIC_SERIALISABLE(uint64_t, 64 / 8);

  C3_UPSILON_IS_STATIC_SERIALISABLE(int8_t, 8 / 8);
  C3_UPSILON_IS_STATIC_SERIALISABLE(int16_t, 16 / 8);
  C3_UPSILON_IS_STATIC_SERIALISABLE(int32_t, 32 / 8);
  C3_UPSILON_IS_STATIC_SERIALISABLE(int64_t, 64 / 8);

#undef C3_UPSILON_IS_SERIALISABLE
  #undef C3_UPSILON_IS_STATIC_SERIALISABLE

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

  inline void squash_static_unsafe(data_ref) {}

  template<typename Head, typename... Tail>
  void squash_static_unsafe(data_ref b, Head&& head, Tail&&... tail) {
    auto len = upsilon::serialised_size<typename remove_all<Head>::type>();
    serialise_static(head, {b.data(), static_cast<data_ref::size_type>(len)});
    if constexpr (sizeof...(Tail) > 0)
      squash_static_unsafe({b.data() + len, static_cast<data_ref::size_type>(b.size() - len)},
                           std::forward<Tail&&>(tail)...);
  }

  template<typename... Input>
  data squash_static(Input&&... in) {
    data ret(total_serialised_size<Input...>());
    squash_static_unsafe(ret, in...);
    return ret;
  }

  template<typename Head, typename... Tail>
  void expand_static_unsafe(data_const_ref b, Head& head, Tail&... tail) {
    head = deserialise<Head>(b);
    if constexpr (sizeof...(Tail) > 0) {
      auto len = serialised_size<Head>();
      expand_static_unsafe({b.data() + len, static_cast<data_ref::size_type>(b.size() - len)},
                           tail...);
    }
  }

  template<typename... Output>
  void expand_static(data_const_ref b, Output&... output) {
    if (total_serialised_size<Output...>() != static_cast<size_t>(b.size()))
      throw std::range_error("Expanding would overrun buffer");
    expand_static_unsafe(b, output...);
  }

  inline void squash_hybrid_unsafe(data_ref) {}

  template<typename Head, typename... Tail>
  inline size_t get_hybrid_prealloc() {
    if constexpr (sizeof...(Tail) == 0)
      return 0;
    else {
      return serialised_size<typename remove_all<Head>::type>() + get_hybrid_prealloc<Tail...>();
    }
  }

  template<typename Head, typename... Tail>
  void squash_hybrid_internal(data& b, size_t offset, Head& head, Tail&... tail) {
    if constexpr (sizeof...(Tail) == 0) {
      data final = serialise(head);
      b.insert(b.end(), final.begin(), final.end());
    }
    else {
      size_t len = serialised_size<typename remove_all<Head>::type>();
      serialise_static(head, {b.data() + offset, static_cast<ssize_t>(len)});
      squash_hybrid_internal(b, offset + len, tail...);
    }
  }

  template<typename... Input>
  data squash_hybrid(Input&&... input) {
    data ret(get_hybrid_prealloc<Input...>());
    squash_hybrid_internal(ret, 0, input...);
    return ret;
  }

  template<typename Head, typename... Tail>
  void expand_hybrid(data_const_ref b, Head& head, Tail&... tail) {
    if constexpr (sizeof...(Tail) == 0) {
      head = deserialise<Head>(b);
    }
    else {
      ssize_t len = static_cast<ssize_t>(serialised_size<Head>());
      head = deserialise<Head>({b.data(), len});
      expand_hybrid({b.data() + len, b.size() - len}, tail...);
    }
  }

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
}

#include "c3/upsilon/data/clean_helpers.hpp"
