#pragma once

#include "c3/nu/data/cstr.hpp"
#include "c3/nu/data/base64.hpp"
#include "c3/nu/data/structs/base.hpp"
#include "c3/nu/types.hpp"
#include "c3/nu/safe_iter.hpp"

#include <iostream>

namespace c3::nu {
  inline std::string json_encode(const obj_struct& ds);

  inline std::string json_encode_datum(obj_struct::value_t t) {
    return std::visit([](auto& x) -> std::string {
      using T = typename remove_all<decltype(x)>::type;

      if constexpr (std::is_same_v<T, std::monostate>)
        return "null";
      else if constexpr (std::is_same_v<T, int64_t>)
        return std::to_string(x);
      else if constexpr (std::is_same_v<T, double>) {
        constexpr size_t buf_size = 64;
        char buf[buf_size] = {0};
        ::snprintf(buf, buf_size, "%g", x);
        return { buf };
      }
      else if constexpr (std::is_same_v<T, std::string>)
        return '"' + cstr_encode(x) + '"';
      else if constexpr (std::is_same_v<T, bool>)
        return x ? "true" : "false";
      else if constexpr (std::is_same_v<T, std::vector<obj_struct>>) {
        std::string acc = "[";
        auto iter = x.begin();
        while (true) {
          acc.append(json_encode(*iter));
          if (++iter == x.end())
            break;
          else
            acc.push_back(',');
        }
        acc.push_back(']');
        return acc;
      }
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
      if (iter != ds.end())
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
    return _is_json_delim(c) || c == '}' || c == ']';
  }
  inline std::string _json_decode_string(safe_iter<std::string_view::const_iterator>& iter) {
    auto str_start = ++iter;
    for (bool is_escaped = false; !(!is_escaped && *iter == '"'); ++iter) {
      if (is_escaped)
        is_escaped = false;
      else if (*iter == '\\')
        is_escaped = true;
    }

    auto str_len = iter - str_start;

    return cstr_decode(std::string_view(&*str_start, str_len));
  }

  inline obj_struct _json_decode_impl(safe_iter<std::string_view::const_iterator>& iter) {
    if (std::isspace(*iter))
      return _json_decode_impl(++iter);
    else if (*begin == '{') {
      if (auto a = begin + 1; a != end && *a == '}')
        return obj_struct::empty_parent();

      obj_struct ret;

      while (true) {
        while (true) {
          if (begin == end)
            throw std::runtime_error("Hit end before key");
          else if (*begin == '"')
            break;
          else
            begin++;
        }
        std::string key = _json_decode_string(begin, end);
        // Iterate to the value
        do if (++begin == end) throw std::runtime_error("Hit end before value");
        while (*begin != ':');
        ++begin;
        // Get the next value
        ret.get_or_add_child(key) = _json_decode_impl(begin, end);
        // Get the next delim
        if (begin == end)
          throw std::runtime_error("Hit end after member");
        while (*begin != ',') {
          if (begin == end)
            throw std::runtime_error("Hit end before delim");
          else if (*begin == '}')
            goto finished_struct;

          ++begin;
        }
      }
      finished_struct:

      ++begin;
      return ret;
    }
    else if (*begin == '[') {
      if (auto a = begin + 1; a != end && *a == ']')
        return obj_struct::arr_t{};

      obj_struct ret;

      while (true) {
        ret.as<obj_struct::arr_t>().push_back(_json_decode_impl(++begin, end));
        if (begin == end)
          throw std::runtime_error("Hit end before value in array");
        while (*begin != ',') {
          if (begin == end)
            throw std::runtime_error("Hit end before delim");
          else if (*begin == ']')
            goto finished_arr;

          ++begin;
        }
      }
      finished_arr:

      ++begin;
      return ret;
    }
    else if (std::isdigit(*begin) || *begin == '-') {
      std::string buf;
      bool is_float = false;
      do {
        if (!isdigit(*begin) && *begin != '-')
          is_float = true;
        buf.push_back(*begin);
      }
      while (++begin != end && !_is_json_eot(*begin));
      if (is_float)
        return { std::stod(buf) };
      else
        return { std::stoll(buf) };
    }
    else if (*begin == '"') {
      return _json_decode_string(begin, end);
    }
    else {
      // We have a literal value
      //
      // Lets grab the next delimiter, and parse the resultant iterator range
      auto lit_start = begin;
      while (!(++begin != end && _is_json_eot(*begin)));

      static const char* true_str = "true";
      static const char* false_str = "false";
      static const char* null_str = "null";

      if (std::equal(lit_start, begin, true_str, true_str + ::strlen(true_str)))
        return true;
      else if (std::equal(lit_start, begin, true_str, false_str + ::strlen(false_str)))
        return false;
      else if (std::equal(lit_start, begin, null_str, null_str + ::strlen(null_str)))
        return nullptr;
      else
        throw std::runtime_error("No valid subparser found");
    }
  }

  inline obj_struct json_decode(std::string_view sv) {
    auto begin = sv.cbegin();
    return _json_decode_impl(begin, sv.cend());
  }
}
