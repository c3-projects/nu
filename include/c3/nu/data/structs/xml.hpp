#pragma once

#include "c3/nu/data/structs/base.hpp"
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
}
