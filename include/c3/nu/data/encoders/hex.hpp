#pragma once

#include <ios>
#include <iomanip>

#include <sstream>

#include "c3/nu/data/base.hpp"

namespace c3::nu {
  template<typename Iter>
  inline void hex_encode_data(std::ostream& os, Iter begin, Iter end) {
    auto pre = os.flags();

    os << std::hex;

    while (begin != end)
      os << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(*begin++);

    os.setf(pre);
  }
  inline void hex_encode_data(std::ostream& os, data_ref b) {
    hex_encode_data(os, b.begin(), b.end());
  }
  template<typename Iter>
  inline std::string hex_encode_data(Iter begin, Iter end) {
    std::ostringstream ss;
    hex_encode_data(ss, begin, end);
    return ss.str();
  }
  inline std::string hex_encode_data(data_ref b) {
    return hex_encode_data(b.begin(), b.end());
  }

  template<typename Iter>
  inline void hex_decode_data(std::istream& is, Iter begin, Iter end) {
    auto pre = is.flags();

    is >> std::hex;

    while (begin != end)
      is >> *begin++;

    is.setf(pre);
  }
  inline void hex_decode_data(std::istream& is, data_ref b) {
    hex_decode_data(is, b.begin(), b.end());
  }
  inline data hex_decode_data(std::istream& is) {
    nu::data ret;

    auto pre = is.flags();

    is >> std::hex;

    while (true) {
      char chars[3] = {0};
      is >> chars[0] >> chars[1];
      // We have to do this after reading, as otherwise we hit a false positive
      if (is.eof()) break;
      ret.emplace_back(::strtoul(chars, nullptr, 16));
    }

    is.setf(pre);

    return ret;
  }
  template<typename Iter>
  inline void hex_decode_data(std::string_view str, Iter begin, Iter end) {
    std::stringstream ss;
    ss << str;
    hex_decode_data(ss, begin, end);
  }
  inline void hex_decode_data(std::string_view str, data_ref b) {
    std::stringstream ss;
    ss << str;
    hex_decode_data(ss, b.begin(), b.end());
  }
  inline data hex_decode_data(std::string_view str) {
    std::stringstream ss;
    ss << str;
    return hex_decode_data(ss);
  }

  template<typename T>
  inline std::string hex_encode(const T& t) {
    auto b = serialise<T>(t);
    return hex_encode_data(b);
  }
  template<typename T>
  inline T hex_decode(std::string_view str) {
    auto b = hex_decode_data(str);
    return deserialise<T>(b);
  }
}
