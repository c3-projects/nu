#pragma once

#include "c3/nu/data/cstr.hpp"
#include "c3/nu/data/base64.hpp"
#include "c3/nu/data/structs/base.hpp"
#include "c3/nu/types.hpp"

#include <iostream>

namespace c3::nu {
  inline std::string json_encode_datum(std::any t) {
    if (auto x = try_any_cast<std::string>(t)) {
      std::string str;
      str.push_back('"');
      str += cstr_encode(*x);
      str.push_back('"');
      return str;
    }
    else {
      std::cout << t.type().name() << std::endl;
      throw std::runtime_error("Could not cast json type");
    }
  }

  inline void _json_encode_impl(const data_struct& ds, std::string& acc) {
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
  inline std::string json_encode(const data_struct& ds) {
    std::string ret;
    _json_encode_impl(ds, ret);
    return ret;
  }
}
