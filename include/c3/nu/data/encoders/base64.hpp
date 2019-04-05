#pragma once

#include "c3/nu/data/base.hpp"
#include "c3/nu/bits.hpp"
#include "c3/nu/integer.hpp"

#include <string>
#include <map>

#include <iostream>
#include <iomanip>

namespace c3::nu {
  static const std::array<char, 64> base64_encode_lookup_table = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
  };

  // TODO: maybe optimise this into a 7 bit datum?
  constexpr std::array<std::optional<bit_datum<6>>, 256> _gen_base64_decode_lookup_table() {
    std::array<std::optional<bit_datum<6>>, 256> ret;

    for (size_t i = 0; i < 64; ++i) {
      ret[static_cast<size_t>(base64_encode_lookup_table[i])] = std::optional<bit_datum<6>>{i};
    }

    return ret;
  }

  static const auto base64_decode_lookup_table = _gen_base64_decode_lookup_table();

  constexpr size_t base64_encoded_len(size_t octets) {
    return 4 * divide_ceil<size_t>(octets, 3);
  }

  constexpr size_t base64_encoded_unpadded_len(size_t octets) {
    return divide_ceil<size_t>(8 * octets, 6);
  }

  constexpr size_t base64_decoded_len(size_t sextets, size_t padding_len) {
    return 3 * (sextets / 4) - padding_len;
  }

  inline std::string base64_encode_data(data_const_ref b) {
    std::string ret(base64_encoded_len(b.size()), '=');

    bits_const_ref bits{b};

    for (size_t i = 0; i < base64_encoded_unpadded_len(b.size()); ++i)
      ret[i] = base64_encode_lookup_table[bits.get_datum<6>(i * 6).get()];

    return ret;
  }

  inline data base64_decode_data(const std::string& str) {
    size_t padding_len = 0;
    {
      auto pos = str.end();

      while (*--pos == '=') ++padding_len;
    }

    if (padding_len > 2)
      throw serialisation_failure("Bad padding on base64 encoded data");

    data ret(base64_decoded_len(str.size(), padding_len));

    bits_ref bits{data_ref{ret}};

    for (size_t i = 0; i < str.size() - padding_len; ++i) {
      if (std::isspace(str[i]))
        continue;
      bits.set_datum(i * 6, base64_decode_lookup_table[str[i]].value());
    }

    return ret;
  }

  template<typename T>
  inline std::string base64_encode(const T& t) {
    return base64_encode_data(serialise(t));
  }

  template<typename T>
  inline T base64_decode(const std::string& str) {
    return deserialise<T>(base64_decode_data(str));
  }
}
