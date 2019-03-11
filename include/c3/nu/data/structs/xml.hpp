#pragma once

#include "c3/nu/data/structs/base.hpp"
#include <regex>

namespace c3::nu {
  inline std::string xml_escape(std::string_view str) {

  }
  inline std::string xml_tag_escape(std::string_view str) {
    std::string ret;
    std::back_insert_iterator out_iter{ret};
    std::regex_replace(ret, str.begin(), str.end(), std::regex(R"(\")"), "&quot;");
    return ret;
  }

  inline std::string _xml_encode_impl(const markup_struct& ms, std::string& str) {
    str.push_back('<');
    str.append(xml_escape(ms.type));

    // Encode attrs
    {
      auto iter = ms.attrs.begin();
      while(true) {
        str.push_back('"');
        str.append(xml_tag_escape(iter->first));
        str.append("\":\"");
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
      std::visit([&](auto x) {
        using U = typename remove_all<decltype(x)>::type;

        if (std::is_same_v<U, markup_struct>)
          _xml_encode_impl(ms, str);
        else xml_escape(str);
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
