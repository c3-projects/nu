#pragma once

#include "c3/nu/data/structs/base.hpp"
#include "c3/nu/safe_iter.hpp"
#include <regex>

namespace c3::nu {
  inline std::string xml_escape(std::string_view str) {
    std::string ret;
    for (auto c : str) {
      if (c == '"')
        ret += "&quot;";
      else ret.push_back(c);
    }
    return ret;
  }
  inline std::string xml_tag_escape(std::string_view str) {
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

  inline void _xml_encode_impl(const markup_struct& ms, std::string& str) {
    str.push_back('<');
    str.append(xml_escape(ms.type));

    // Encode attrs
    {
      auto iter = ms.attrs.begin();
      if (iter != ms.attrs.end())
        while(true) {
          str.push_back('"');
          str.append(xml_tag_escape(iter->first));
          str.append(R"(":")");
          str.append(xml_tag_escape(iter->second));
          str.push_back('"');
          if (++iter == ms.attrs.end())
            break;
          else
            str.push_back(' ');
        }
    }
    str.push_back('>');

    for (auto& i : ms) {
      std::visit([&](auto& x) {
        using U = typename remove_all<decltype(x)>::type;

        if constexpr (std::is_same_v<U, markup_struct>)
          _xml_encode_impl(x, str);
        else
          str.append(xml_escape(x));
      },i);
    }

    str.append("</");
    str.append(xml_escape(ms.type));
    str.push_back('>');
  }

  inline std::string xml_encode(const markup_struct& ms) {
    std::string ret;

    _xml_encode_impl(ms, ret);

    return ret;
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
        while (*++iter != ' ');
        throw hit_elem_end({type_begin, iter});
      }

      markup_struct ret;
      {
        auto type_begin = ++iter;
        while (*++iter != ' ');
        ret.type = {type_begin, iter};
        while (*++iter != '>');
        iter++;
      }

      // TODO: enumerate attrs
      // TODO: check for "/>"

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
