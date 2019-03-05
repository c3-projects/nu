#pragma once

#include "c3/nu/data/cstr.hpp"
#include "c3/nu/data/base64.hpp"
#include "c3/nu/data/structs/base.hpp"
#include "c3/nu/types.hpp"

#include <iostream>

namespace c3::nu {

  inline std::string json_encode_datum(struct_value_t<obj_struct> t) {
    return std::visit([](auto& x) -> std::string {
      using T = typename remove_all<decltype(x)>::type;

      if constexpr (std::is_same_v<T, std::monostate>)
        return "{}";
      //else if constexpr (std::is_same_v<T, nu::bigint>)
      //  return std::to_string(x);
      else if constexpr (std::is_same_v<T, std::string>)
        return '"' + cstr_encode(x) + '"';
      else {
        std::cout << typeid(T).name() << std::endl;
        throw std::runtime_error("Could not cast json type");
      }
    }, t);
  }

  inline void _json_encode_impl(const obj_struct& ds, std::string& acc) {
    if (ds.is_parent()) {
      acc.push_back('{');
      auto iter = ds.begin();
      while (true) {
        acc.push_back('"');
        acc.append(cstr_encode(iter->first));
        acc.append("\":");
        _json_encode_impl(iter->second, acc);
        if (++iter == ds.end())
          break;
        else
          acc.push_back(',');
      }
      acc.push_back('}');
    }
    else {
      acc += json_encode_datum(ds.get_value());
    }
  }
  inline std::string json_encode(const obj_struct& ds) {
    std::string ret;
    _json_encode_impl(ds, ret);
    return ret;
  }
}
