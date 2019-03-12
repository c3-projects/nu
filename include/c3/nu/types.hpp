#pragma once

#include <type_traits>
#include <any>
#include <optional>
#include <variant>

#include "c3/nu/data/span_deps.hpp"
#include "c3/nu/sfinae.hpp"
#include "c3/nu/integer.hpp"

namespace c3::nu {
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
