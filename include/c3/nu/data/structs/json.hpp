#pragma once

#include "c3/nu/data/cstr.hpp"
#include "c3/nu/data/structs/base.hpp"

namespace c3::nu {
  template<typename T>
  inline std::string json_encode_datum(const T& t);
  inline void _json_encode_impl(const data_struct& ds, std::string& acc) {
    if (ds.is_parent()) {
      acc.push_back('{');
      auto iter = ds.begin();
      while (true) {
        acc.append(cstr_encode(iter->first));
        acc.push_back(':');
        _json_encode_impl(iter->second, acc);
        if (++iter == ds.end())
          continue;
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
