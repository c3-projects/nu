#pragma once

#include <cstdint>
#include <cstdlib>
#include <limits>

#include <ostream>

#include "c3/nu/data.hpp"
#include "c3/nu/integral.hpp"

#include "c3/nu/int_maths.hpp"

#include <climits>

//! This should work regardless of byte size, but be warned that I have only tested it on octet bytes

namespace c3::nu {
  /// The smallest addressible chunk of information
  using byte_t = unsigned char;

  constexpr size_t __MAX_BIT_DATUM_SIZE = 64;
  using n_bits_rep_t = integral_fast_upto_t<__MAX_BIT_DATUM_SIZE>;
  constexpr n_bits_rep_t MAX_BIT_DATUM_SIZE = static_cast<n_bits_rep_t>(__MAX_BIT_DATUM_SIZE);

  template<bool> struct Range;

  template<n_bits_rep_t Bits = dynamic_size>
  class bit_datum;

  class bits_const_ref {
  public:
    size_t BITS() const noexcept { return _bits; };

  private:
    const byte_t* _ptr;
    size_t _bits;

  public:
    constexpr size_t safe_access_bytes() const { return divide_ceil<size_t>(_bits, CHAR_BIT); }
    constexpr size_t n_full_bytes() const { return _bits / CHAR_BIT; }
    constexpr size_t n_final_bits() const { return _bits % CHAR_BIT; }

  public:
    constexpr bool get_bit(size_t pos) const noexcept {
      return pos < _bits && (_ptr[pos / CHAR_BIT] & (1 << (CHAR_BIT - 1 - pos % CHAR_BIT))) != 0;
    }
    template<n_bits_rep_t Bits>
    constexpr bit_datum<Bits> get_datum(size_t pos) const noexcept;
    constexpr bit_datum<dynamic_size> get_datum(size_t pos, n_bits_rep_t bits) const noexcept;
    constexpr uint8_t get_byte(size_t pos) const noexcept;

  public:
    constexpr bits_const_ref(decltype(_ptr) ptr, decltype(_bits) bits) :
      _ptr{ptr}, _bits{bits} {}
    constexpr bits_const_ref(data_const_ref b) :
      _ptr{b.data()}, _bits{static_cast<size_t>(b.size() * CHAR_BIT)} {}
  };

  class bits_ref {
  public:
    size_t BITS() const noexcept { return _bits; };

  private:
    byte_t* _ptr;
    size_t _bits;

  public:
    constexpr size_t safe_access_bytes() const { return divide_ceil<size_t>(_bits, CHAR_BIT); }
    constexpr size_t n_full_bytes() const { return _bits / CHAR_BIT; }
    constexpr size_t n_final_bits() const { return _bits % CHAR_BIT; }

  public:
    constexpr bool get_bit(size_t pos) const noexcept {
      return pos < _bits && (_ptr[pos / CHAR_BIT] & (1 << (CHAR_BIT - 1 - pos % CHAR_BIT))) != 0;
    }
    constexpr void set_bit(size_t pos) noexcept {
      if (pos < _bits) (_ptr[pos / CHAR_BIT] |= (1 << (CHAR_BIT - 1 - pos % CHAR_BIT)));
    }
    constexpr void clear_bit(size_t pos) noexcept {
      if (pos < _bits) (_ptr[pos / CHAR_BIT] ^= (1 << (CHAR_BIT - 1 - pos % CHAR_BIT)));
    }
    template<n_bits_rep_t Bits>
    constexpr bit_datum<Bits> get_datum(size_t pos) const noexcept;
    constexpr bit_datum<dynamic_size> get_datum(size_t pos, n_bits_rep_t bits) const noexcept;

    template<n_bits_rep_t Bits>
    constexpr void set_datum(size_t pos, bit_datum<Bits> b) noexcept;

    constexpr byte_t get_byte(size_t pos) const noexcept;

  public:
    constexpr bits_ref(decltype(_ptr) ptr, decltype(_bits) bits) :
      _ptr{ptr}, _bits{bits} {}
    constexpr bits_ref(data_ref b) :
      _ptr{b.data()}, _bits{static_cast<size_t>(b.size() * CHAR_BIT)} {}
  };

  template<n_bits_rep_t Bits, typename = Range<true>>
  class bit_datum_rep;

  template<n_bits_rep_t Bits>
  class bit_datum_rep<Bits, Range<(Bits >= 1 && Bits <= 8)>> {
  public:
    using type = uint8_t;
    static constexpr type Mask = std::numeric_limits<type>::max() >> (CHAR_BIT - 1 - Bits);
  };

  template<n_bits_rep_t Bits>
  class bit_datum_rep<Bits, Range<(Bits > 8 && Bits <= 16)>> {
  public:
    using type = uint16_t;
    static constexpr type Mask = std::numeric_limits<type>::max() >> (15 - Bits);
  };

  template<n_bits_rep_t Bits>
  class bit_datum_rep<Bits, Range<(Bits > 16 && Bits <= 32)>> {
  public:
    using type = uint32_t;
    static constexpr type Mask = std::numeric_limits<type>::max() >> (31 - Bits);
  };

  template<n_bits_rep_t Bits>
  class bit_datum_rep<Bits, Range<(Bits > 32 && Bits <= 64)>> {
  public:
    using type = uint64_t;
    static constexpr type Mask = std::numeric_limits<type>::max() >> (63 - Bits);
  };

  template<n_bits_rep_t Bits>
  class bit_datum {
    // Upper bound allows iteration by 2 in on_off
    // TODO: check I haven't been too cautious
    static_assert(Bits > 0 && Bits < MAX_BIT_DATUM_SIZE, "bit_datum size out of range");

  public:
    using rep_t = typename bit_datum_rep<Bits>::type;
    constexpr n_bits_rep_t BITS() const noexcept { return Bits; };

  private:
    rep_t _value;

  public:
    constexpr bit_datum<Bits>& safe_set(rep_t new_val) {
      _value = new_val & bit_datum_rep<Bits>::Mask;
      return *this;
    }
    constexpr bit_datum<Bits>& unsafe_set(rep_t new_val) {
      _value = new_val;
      return *this;
    }
    constexpr bit_datum<Bits>& operator=(rep_t new_val) {
      return safe_set(new_val);
    }
    constexpr bool get_bit(size_t pos) const {
      return pos < Bits && (_value & (1 << (Bits - pos - 1))) != 0;
    }
    constexpr void set_bit(size_t pos) {
      if (pos < Bits) (_value |= (1 << (Bits - pos - 1)));
    }
    constexpr void clear_bit(size_t pos) {
      if (pos < Bits) (_value ^= (1 << (Bits - pos - 1)));
    }

    constexpr rep_t get() const { return _value; }

    constexpr operator rep_t() const { return get(); }
    constexpr rep_t operator*() { return get(); }

    inline operator bits_ref() { return { reinterpret_cast<byte_t*>(&_value), Bits }; }

    inline operator bit_datum<dynamic_size>() const;

  public:
    constexpr bit_datum() : _value{0} {}
    constexpr bit_datum(decltype(_value) value) : _value{value} {}
    static constexpr bit_datum<Bits> all_set() { return bit_datum_rep<Bits>::Mask; }
    static constexpr bit_datum<Bits> on_off() {
      bit_datum<Bits> ret;
      for (n_bits_rep_t pos = 0; pos < Bits; pos += 2)
        ret.set_bit(pos);
      return ret;
    }
    static constexpr bit_datum off_on() {
      bit_datum<Bits> ret;
      for (n_bits_rep_t pos = 1; pos < Bits; pos += 2)
        ret.set_bit(pos);
      return ret;
    }

    static constexpr size_t split_len(size_t n_bytes) {
      return divide_ceil(n_bytes * CHAR_BIT, Bits);
    }
    static constexpr size_t combine_len(size_t n_data) {
      return divide_ceil<size_t>(n_data * Bits, CHAR_BIT);
    }

    static constexpr void split(data_const_ref in, gsl::span<bit_datum<Bits>> out) {
      bits_const_ref b(in);

      for (size_t i = 0; i < static_cast<size_t>(out.size()); ++i)
        out[static_cast<ssize_t>(i)] = b.get_datum<Bits>(i*Bits);
    }
    static inline std::vector<bit_datum<Bits>> split(data_const_ref in) {
      size_t n_data = split_len(static_cast<size_t>(in.size()));

      std::vector<bit_datum<Bits>> ret(n_data);
      split(in, ret);

      return ret;
    }

    static constexpr void combine(gsl::span<const bit_datum<Bits>> in,
                                  data_ref out,
                                  size_t offset = 0) {
      typename decltype(in)::index_type in_pos = offset / Bits;
      size_t in_offset = offset % Bits;

      for (auto& out_byte : out) {
        for (size_t i = 0; i < CHAR_BIT; ++i) {
          if (in[in_pos].get_bit(in_offset++))
            out_byte |= (1 << (CHAR_BIT - 1 - i));
          if (in_offset == Bits) {
            in_offset = 0;
            ++in_pos;
            if (in_pos == in.size()) return;
          }
        }
      }
    }
    static inline data combine(gsl::span<const bit_datum<Bits>> in, size_t offset = 0) {
      size_t n_data = combine_len(in.size() - offset / Bits);

      data ret(n_data);
      combine(in, ret, offset);

      return ret;
    }
  };

  template<>
  class bit_datum<dynamic_size> {
  public:
    using rep_t = uint64_t;
    constexpr n_bits_rep_t BITS() const noexcept { return _bits; };

  private:
    rep_t _value;
    n_bits_rep_t _bits;

  public:
    constexpr bit_datum<dynamic_size>& safe_set(rep_t new_val) {
      _value = new_val & (std::numeric_limits<rep_t>::max() >> (63 - _bits));
      return *this;
    }
    constexpr bit_datum<dynamic_size>& unsafe_set(rep_t new_val) {
      _value = new_val;
      return *this;
    }
    constexpr bit_datum<dynamic_size>& operator=(rep_t new_val) {
      safe_set(new_val);
      return *this;
    }
    constexpr bool get_bit(size_t pos) const {
      return pos < _bits && (_value & (1 << (_bits - pos - 1))) != 0;
    }
    constexpr void set_bit(size_t pos) {
      if (pos < _bits) (_value |= (1 << (_bits - pos - 1)));
    }
    constexpr void clear_bit(size_t pos) {
      if (pos < _bits) (_value ^= (1 << (_bits - pos - 1)));
    }

    constexpr rep_t get() const { return _value; }

    constexpr operator rep_t() const { return get(); }
    constexpr rep_t operator*() { return get(); }

    inline operator bits_ref() { return { reinterpret_cast<byte_t*>(&_value), _bits }; }

  public:
    constexpr bit_datum(decltype(_bits) bits) : _value{0}, _bits{bits} {
      if (bits > MAX_BIT_DATUM_SIZE)
        throw std::range_error("bit_datum size out of range");
    }
    constexpr bit_datum(decltype(_value) value, decltype(_bits) bits) : _value{value}, _bits{bits} {
      if (bits > MAX_BIT_DATUM_SIZE)
        throw std::range_error("bit_datum size out of range");
    }

    static constexpr bit_datum on_off(decltype(_bits) bits) {
      bit_datum<dynamic_size> ret(bits);
      for (n_bits_rep_t pos = 0; pos < bits; pos += 2)
        ret.set_bit(pos);
      return ret;
    }
    static constexpr bit_datum off_on(decltype(_bits) bits) {
      bit_datum<dynamic_size> ret(bits);
      for (n_bits_rep_t pos = 1; pos < bits; pos += 2)
        ret.set_bit(pos);
      return ret;
    }

    static constexpr size_t split_len(size_t n_bytes, n_bits_rep_t bits) {
      return divide_ceil(n_bytes * CHAR_BIT, bits);
    }
    static constexpr size_t combine_len(size_t n_data, n_bits_rep_t bits) {
      return divide_ceil<size_t>(n_data * bits, CHAR_BIT - 1);
    }

    static constexpr void split(data_const_ref in, gsl::span<bit_datum<dynamic_size>> out, n_bits_rep_t bits) {
      bits_const_ref b(in);

      for (size_t i = 0; i < static_cast<size_t>(out.size()); ++i)
        out[static_cast<ssize_t>(i)] = b.get_datum(i * bits, bits);
    }
    static inline std::vector<bit_datum<dynamic_size>> split(data_const_ref in, n_bits_rep_t bits) {
      size_t n_data = split_len(static_cast<size_t>(in.size()), bits);

      std::vector<bit_datum<dynamic_size>> ret(n_data, bit_datum<dynamic_size>(bits));
      split(in, ret, bits);

      return ret;
    }

    static constexpr void combine(gsl::span<const bit_datum<dynamic_size>> in,
                                  data_ref out,
                                  n_bits_rep_t bits,
                                  size_t offset = 0) {
      decltype(in)::index_type in_pos = offset / CHAR_BIT;
      size_t in_offset = offset % CHAR_BIT;

      for (auto& out_byte : out) {
        for (size_t i = 0; i < CHAR_BIT; ++i) {
          if (in[in_pos].get_bit(++in_offset))
            out_byte |= (1 << (CHAR_BIT - 1 - i));
          if (in_offset == bits) {
            in_offset = 0;
            ++in_pos;
            if (in_pos == in.size()) return;
          }
        }
      }
    }
    static inline data combine(gsl::span<const bit_datum<dynamic_size>> in,
                               n_bits_rep_t bits,
                               size_t offset = 0) {
      size_t n_data = combine_len(static_cast<size_t>(in.size()) - offset / bits, bits);

      data ret(n_data);
      combine(in, ret, bits, offset);

      return ret;
    }
  };

  template<n_bits_rep_t Bits>
  bit_datum<Bits>::operator bit_datum<dynamic_size>() const { return { _value, Bits }; }

  template<n_bits_rep_t Bits>
  constexpr bit_datum<Bits> bits_ref::get_datum(size_t pos) const noexcept {
    bit_datum<Bits> ret;

    for (n_bits_rep_t i = 0; i < Bits; ++i)
      if (get_bit(pos + i))
        ret.set_bit(i);

    return ret;
  }

  constexpr bit_datum<dynamic_size> bits_ref::get_datum(size_t pos, n_bits_rep_t bits) const noexcept {
    bit_datum<dynamic_size> ret(bits);

    for (n_bits_rep_t i = 0; i < bits; ++i)
      if (get_bit(pos + i))
        ret.set_bit(i);

    return ret;
  }

  constexpr byte_t bits_ref::get_byte(size_t pos) const noexcept{
    return get_datum<CHAR_BIT>(pos);
  }

  template<n_bits_rep_t Bits>
  constexpr bit_datum<Bits> bits_const_ref::get_datum(size_t pos) const noexcept {
    bit_datum<Bits> ret;

    for (n_bits_rep_t i = 0; i < Bits; ++i)
      if (get_bit(pos + i))
        ret.set_bit(i);

    return ret;
  }

  constexpr bit_datum<dynamic_size> bits_const_ref::get_datum(size_t pos, n_bits_rep_t bits) const noexcept {
    bit_datum<dynamic_size> ret(bits);

    for (n_bits_rep_t i = 0; i < bits; ++i)
      if (get_bit(pos + i))
        ret.set_bit(i);

    return ret;
  }
  constexpr byte_t bits_const_ref::get_byte(size_t pos) const noexcept {
    return get_datum<CHAR_BIT>(pos);
  }

  template<n_bits_rep_t Bits>
  constexpr void bits_ref::set_datum(size_t pos, bit_datum<Bits> b) noexcept {
    for (size_t i = 0; i < b.BITS(); ++i)
      if (b.get_bit(i))
        set_bit(pos + i);
  }

  template<n_bits_rep_t Bits>
  inline std::ostream& operator<<(std::ostream& os, bit_datum<Bits> n) {
    for(n_bits_rep_t i = 0; i < n.BITS(); ++i)
      os << (n.get_bit(i) ? 1 : 0);
    return os;
  }
}
