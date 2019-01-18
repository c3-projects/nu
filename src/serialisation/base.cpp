#include "c3/upsilon/data/base.hpp"
#include "c3/upsilon/data/common.hpp"

#include <algorithm>
#include <sstream>
#include <iomanip>

#include <botan/base64.h>

namespace c3::upsilon {
  template<typename Int>
  uint8_t log8_int(Int i) {
    static_assert(sizeof(i) < std::numeric_limits<Int>::max(), "log8 of int could cause overflow");
    uint8_t count = 0;

    // 1v1 me
    for (; i != 0; i >>= 8, ++count);
    return count;
  }

  data serialise_dynamic_struct(input_buffers input) {
    data ret;

    for (auto buffer : input) {
      // Making this fixed size for ease of prpocessing
      // also I'm going to screw up the bounds checking somewhere
      //
      // When we send 4 gigabytes of information in one struct, then we change the version number
      uint32_t len = static_cast<uint32_t>(buffer.size());
      data len_b = serialise(len);

      ret.insert(ret.end(), len_b.begin(), len_b.end());
      ret.insert(ret.end(), buffer.begin(), buffer.end());
    }

    return ret;
  }

  std::vector<data_const_ref> deserialise_dynamic_struct(data_const_ref input) {
    std::vector<data_const_ref> ret;

    auto pos = input.begin();

    while (pos != input.end()) {
      auto len_begin = pos;
      len_begin += static_cast<ssize_t>(serialised_size<uint32_t>());

      uint32_t len = deserialise<uint32_t>({ &*len_begin, &*pos });

      ret.push_back(data_const_ref{&*pos, len});

      pos += len;
    }

    return ret;
  }

  std::string base64_encode_data(data_const_ref b) {
    return Botan::base64_encode(b.data(), b.size());
  }
  data base64_decode_data(const std::string& str) {
    data ret(Botan::base64_decode_max_output(str.size()));
    ret.resize(Botan::base64_decode(ret.data(), str.data()), str.size());
    return ret;
  }
}
