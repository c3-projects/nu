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
      else if constexpr (std::is_same_v<T, int64_t>)
        return std::to_string(x);
      else if constexpr (std::is_same_v<T, double>) {
        constexpr size_t buf_size = 64;
        char buf[buf_size];
        ::snprintf(buf, buf_size, "%g", x);
        return { buf };
      }
      else if constexpr (std::is_same_v<T, std::string>)
        return '"' + cstr_encode(x) + '"';
      else if constexpr (std::is_same_v<T, bool>)
        return x ? "true" : "false";
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
  inline bool _is_json_delim(char c) {
    return c == ',' || std::isspace(c);
  }
  inline bool _is_json_eot(char c) {
    return _is_json_delim(c) || c == '}';
  }
  inline obj_struct _json_decode_impl(std::string_view::const_iterator& begin, std::string_view::const_iterator end) {
    if (begin == end)
      throw std::runtime_error("Hit end");

    if (std::isspace(*begin))
      return _json_decode_impl(++begin, end);
    else if (*begin == '{') {
      obj_struct ret;

      while (true) {
        if (++begin == end)
          throw std::runtime_error("Unterminated bracket");

        _json_decode_impl(begin, end);
        // Get the next delim
        while (++begin != end && !_is_json_delim(*begin));
      }
    }
    else if (std::isdigit(*begin)) {
      std::string buf;
      do {
        buf.push_back(*begin);
      }
      while (++begin != end && !_is_json_eot(*begin));
      return std::stod(buf);
    }
    else if (*begin == '"') {
      std::string buf;
      auto str_start = ++begin;
      bool is_escaped = false;
      {
        if (*begin == '"' && !is_escaped)
          break;

        if (is_escaped)
        buf.push_back(*begin);
      }
      while (++begin != end && !_is_json_eot(*begin))

      return std::stod(buf);
    }
    else {
      throw std::runtime_error("No valid subparser found");
    }
  }

  inline obj_struct json_decode(std::string_view sv) {
    auto begin = sv.cbegin();
    return _json_decode_impl(begin, sv.cend());
  }
}
