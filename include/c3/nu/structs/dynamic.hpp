#pragma once

#include "c3/nu/data.hpp"

namespace c3::nu {
  template<typename SizeType, typename... Input>
  inline data squash_dynamic(Input&&... input) {
    // First we serialise all of the inputs separately
    std::vector<data> serialised_buffers(sizeof...(input));

    serialise_all(serialised_buffers, input...);

    data ret((sizeof...(input) - 1) * serialised_size<SizeType>());

    // Then we compute all of the lengths of the buffers to form the header
    // We do not need the last, as that can be inferred by the buffer size
    for (size_t i = 0; i < serialised_buffers.size() - 1; ++i) {
      auto begin = ret.data() + serialised_size<SizeType>() * i;

      SizeType val = serialised_buffers[i].size();

      serialise_static(val, { begin, serialised_size<SizeType>() });
    }

    // Finally we add the serialised values calculated at the start to the buffer
    // With the header, we can reconstruct the boundaries between the individual elements
    for (auto& i : serialised_buffers)
      ret.insert(ret.end(), i.begin(), i.end());

    return ret;
  }

  template<typename Head, typename... Tail>
  inline void _expand_dynamic_internal(data_const_ref b,
                                       gsl::span<const size_t> sizes,
                                       Head& head,
                                       Tail&... tail) {
    if constexpr (sizeof...(Tail) == 0)
        head = deserialise<Head>(b);
    else {
      head = deserialise<Head>(b.subspan(0, sizes[0]));
      _expand_dynamic_internal(b.subspan(sizes[0]), sizes.subspan(1), tail...);
    }
  }

  template<typename SizeType, typename... Output>
  inline void expand_dynamic(data_const_ref b, Output&... output) {
    static_assert(std::is_trivially_destructible_v<SizeType>,
                  "SizeType must be trivially destructible");

    // First we parse the header
    std::array<SizeType, sizeof...(Output) - 1> header;
    if (static_cast<size_t>(b.size()) < serialised_size<decltype(header)>())
      throw serialisation_failure("The input buffer was too small");

    data_const_ref header_data{b.data(), serialised_size<decltype(header)>()};

    header = deserialise<SizeType, sizeof...(Output) - 1>(header_data);

    size_t total_data_len_bar_one = 0;
    for (auto& i : header)
      if (!try_add(total_data_len_bar_one, i))
        throw std::runtime_error("Cannot store entire dynamic_struct in memory");

    // Check that the data boundary makes sense
    if (total_data_len_bar_one > b.size() - serialised_size<decltype(header)>())
      throw std::invalid_argument("Dynamic struct's header is inconsistent with its buffer");

    // Since we could add all the data up, we do not need to worry about overflow
    std::array<size_t, sizeof...(Output) - 1> friendly_header;
    std::copy(header.begin(), header.end(), friendly_header.begin());

    auto payload = b.subspan(serialised_size<decltype(header)>());

    // Shove the data into the variables
    _expand_dynamic_internal(payload, friendly_header, output...);
  }
}
