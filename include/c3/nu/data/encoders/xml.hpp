#pragma once

#include "c3/nu/data/encoders/base.hpp"
#include "c3/nu/safe_iter.hpp"
#include "c3/nu/integer.hpp"
#include <regex>

namespace c3::nu {
  inline std::string xml_string_escape(std::string_view str) {
    std::string ret;
    for (auto c : str) {
      if (c == '"')
        ret += "&quot;";
      else ret.push_back(c);
    }
    return ret;
  }
  inline std::string xml_string_unescape(std::string_view str) {
    std::string ret;
    for (auto c : str) {
      if (c == '"')
        ret += "&quot;";
      else ret.push_back(c);
    }
    return ret;
  }
  inline std::string xml_escape(std::string_view str) {
    std::string ret;
    for (auto c : str) {
      if (c == '&')
        ret += "&amp;";
      else if (c == '<')
        ret += "&lt;";
      else if (c == '>')
        ret += "&gt;";
      else if (c == '"')
        ret += "&quot;";
      else if (c == '\'')
        ret += "&apos;";
      else ret.push_back(c);
    }
    return ret;
  }
  inline std::string xml_unescape(std::string_view str) {
    std::string ret;

    for (auto iter = safe(str).begin(); !iter.is_end(); ++iter) {
      if (*iter == '&') {
        auto* seq_begin = &*iter;
        while (*++iter != ';');
        auto seq_end = &*iter;

        std::string_view sv(seq_begin, seq_end - seq_begin);

        if (sv == "amp")
          ret.push_back('&');
        else if (sv == "lt")
          ret.push_back('<');
        else if (sv == "gt")
          ret.push_back('>');
        else if (sv == "quot")
          ret.push_back('\"');
        else if (sv == "apos")
          ret.push_back('\'');
        else
          throw std::runtime_error("Invalid xml escape");
      }
      else ret.push_back(*iter);
    }
    return ret;
  }

  inline bool xml_verify_name(std::string_view name) {
    // Adapted from w3 xml spec 2.3

    // Sorry
    static const std::regex name_regex (
      // NameStartChar:
      "^"
      "["
        R"(:A-Z_a-z\u00C0-\u00D6\u00D8-\u00F6\u00F8-\u02FF)"
        R"(\u0370-\u037D\u037F-\u1FFF\u200C-\u200D\u2070-\u218F\u2C00-\u2FEF)"
        R"(\u3001-\uD7FF\uF900-\uFDCF\uFDF0-\uFFFD\u10000-\uEFFFF)"
      "]"

      // NameChar is above with some extras
      "["
        R"(:A-Z_a-z\u00C0-\u00D6\u00D8-\u00F6\u00F8-\u02FF)"
        R"(\u0370-\u037D\u037F-\u1FFF\u200C-\u200D\u2070-\u218F\u2C00-\u2FEF)"
        R"(\u3001-\uD7FF\uF900-\uFDCF\uFDF0-\uFFFD\u10000-\uEFFFF)"
        R"(-\.0-9\u00B7\u0300-\u036F\u203F-\u2040)"
      "]*"
      "$", std::regex::optimize | std::regex::extended
    );

    return std::regex_match(name.begin(), name.end(), name_regex);
  }

  namespace detail {
    inline void xml_encode_impl(const markup_struct& ms, std::string& str) {
      str.push_back('<');
      if (!xml_verify_name(ms.type))
        throw std::runtime_error("Invalid name for XML element");
      str.append(ms.type);

      // Encode attrs
      {
        if (auto iter = ms.attrs.begin(); iter != ms.attrs.end()) {
          do {
            str.push_back(' ');

            if (!xml_verify_name(iter->first))
              throw std::runtime_error("Invalid name for XML attribute");
            str.append(iter->first);
            str.append(R"(=")");
            str.append(xml_string_escape(iter->second));
            str.push_back('"');
          }
          while (++iter != ms.attrs.end());
        }
      }
      str.push_back('>');

      for (auto& i : ms) {
        std::visit([&](auto& x) {
          using U = typename remove_all<decltype(x)>::type;

          if constexpr (std::is_same_v<U, markup_struct>)
            xml_encode_impl(x, str);
          else
            str.append(xml_escape(x));
        },i);
      }

      str.append("</");
      str.append(xml_escape(ms.type));
      str.push_back('>');
    }
  }

  inline std::string xml_encode(const markup_struct& ms) {
    std::string ret;

    detail::xml_encode_impl(ms, ret);

    return ret;
  }

  inline std::string xml_encode(std::string_view value) {
    return xml_string_escape(value);
  }

  inline std::string xml_encode(const markup_struct::value_t& v) {
    return std::visit([](auto& x) -> std::string {
      if constexpr (std::is_same_v<typename remove_all<decltype(x)>::type, markup_struct>) {
        return xml_encode(x);
      }
      else {
        return xml_encode(std::string_view(x));
      }
    }, v);
  }

  inline markup_struct::value_t _xml_decode_impl(safe_iter<std::string_view::iterator>& iter) {
    class hit_elem_end {
    public:
      std::string type;

    public:
      inline hit_elem_end(decltype(type)&& type) : type(std::move(type)) {}
    };

    // TODO: handle xml:space
    while (std::isspace(*iter)) ++iter;

    if (*iter == '<') {
      ++iter;
      if (*iter == '/') {
        auto type_begin = ++iter;
        while (!std::isspace(*++iter) && *iter != '>');
        auto type_end = iter;
        ++iter;
        throw hit_elem_end({type_begin, type_end});
      }

      markup_struct ret;
      {
        auto type_begin = iter;
        while (*++iter != ' ' && *iter != '>');
        ret.type = {type_begin, iter};
      }

      {
        while(*iter != '/' && *iter != '>') {
          while (std::isspace(*iter)) ++iter;
          auto attr_name_begin = iter;
          while (*iter != '=') ++iter;
          auto attr_name_end = iter;
          ++iter;
          while (std::isspace(*iter)) ++iter;
          auto quote_type = *iter;
          if (quote_type != '\'' && quote_type != '"')
            throw std::runtime_error("Bad quote type");
          ++iter;
          auto attr_value_begin = iter;
          while (*iter != quote_type) ++iter;
          auto attr_value_end = iter;
          ++iter;

          ret.get_or_create_attr({attr_name_begin, attr_name_end}) =
              xml_unescape(std::string_view{&*attr_value_begin,
                            int_cast<size_t>(attr_value_end - attr_value_begin)});
        }
      }

      if (*iter == '/') {
        if (*++iter != '>')
          throw std::runtime_error("Bad tag closer");
        else
          return ret;
      }

      while (*iter != '>') ++iter;

      ++iter;

      try {
        while(true)
          ret.add(_xml_decode_impl(iter));
      }
      catch (hit_elem_end e) {
        if (e.type != ret.type)
          throw std::runtime_error("Mismatched tags");
      }

      return ret;
    }
    else {
      auto str_begin = iter;
      while (*iter != '<' && !iter.is_end())
        ++iter;

      return std::string{str_begin, iter};
    }
  }

  inline markup_struct xml_decode(std::string_view str) {
    auto iter = safe(str).begin();
    return std::get<markup_struct>(_xml_decode_impl(iter));
  }
}
