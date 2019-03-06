#pragma once

#include <type_traits>
#include <any>
#include <optional>
#include <variant>

#include "c3/nu/data/span_deps.hpp"

namespace c3::nu {
  template<class T, class U=
    typename std::remove_cv<
    typename std::remove_pointer<
    typename std::remove_reference<
    T
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

  template<typename Base>
  inline std::optional<Base> try_any_move_cast(std::any&& a) {
    try {
      std::any_cast<Base>(std::move(a));
    } catch (std::bad_any_cast) {
      return std::nullopt;
    }
  }
  template<typename Base>
  inline std::optional<Base> try_any_cast(const std::any& a) {
    try {
      return std::any_cast<Base>(a);
    } catch (std::bad_any_cast) {
      return std::nullopt;
    }
  }

  template<typename T, typename... Types>
  T& get_or_create_alternative(std::variant<Types...>& v) {
    if (std::holds_alternative<std::monostate>(v)) {
      T& ret = v.template emplace<T>();
      return ret;
    }
    else
      return std::get<T>(v);
  }
}
