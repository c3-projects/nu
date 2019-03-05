#pragma once

#include <vector>

#include "c3/nu/data.hpp"

namespace c3::nu {
  class bigint {
  private:
    std::vector<uint8_t> _data;
    bool _sign;

  private:
    inline void _add_op(const bigint& other) {
      _data.resize(std::max(_data.size(), other._data.size()), 0);

      uint8_t carry = false;

      auto a = _data.begin();
      for (auto b = other._data.begin(); a < _data.end(); ++a, ++b) {
        // Unsigned overflow is well-defined in c++, so we do not need to worry about it
        uint8_t old_carry = carry;

        *a += *b;
        carry = (*a < *b);

        *a += old_carry;
      }

      if (carry)
        _data.push_back(1);
    }

    inline void _sub_op(const bigint& other) {
      uint8_t carry = false;

      _data.resize(std::max(_data.size(), other._data.size()), 0);

      auto sub_start = _data.rbegin() + (_data.size() - other._data.size());

      if (*this >= other) {
        auto a = sub_start;
        for (auto b = other._data.rbegin(); a != _data.rend(); ++a, ++b) {
          // Unsigned overflow is well-defined in c++, so we do not need to worry about it
          uint8_t old_carry = carry;

          carry = (*a < *b);
          *a -= *b;

          *a -= old_carry;
        }

        if (carry)
          *--sub_start -= 1;
      }
      else {
        auto a = sub_start;
        for (auto b = other._data.rbegin(); a != _data.rend(); ++a, ++b) {
          // Unsigned overflow is well-defined in c++, so we do not need to worry about it
          uint8_t old_carry = carry;

          carry = (*a > *b);
          *a = *b - *a;

          *a -= old_carry;
        }

        if (carry)
          *--sub_start -= 1;

        _sign = !_sign;
      }

      clean();
    }

    inline bigint _mul_op(const bigint& other) {
      throw std::runtime_error("_mul_op not implemented");
      _sign = (_sign == other._sign);

      auto operand = *this;

      *this = 0;

      return remainder;
    }

    inline bigint _div_op(const bigint& other) {
      _sign = (_sign == other._sign);
      if (other > *this) {
        auto remainder = std::move(*this);
        *this = 0;
        return remainder;
      }
      throw std::runtime_error("_div_op not implemented");

      bigint remainder = 0;

      return remainder;
    }

  public:
    template<typename T>
    constexpr bool can_convert() const noexcept {
      return serialised_size<T>() >= _data.size();
    }

  private:
    inline void clean() {
      while (_data.size() > 0 && _data.back() == 0)
        _data.pop_back();
    }

  public:
    constexpr bool is_zero() const noexcept { return _data.empty(); }

  public:
    inline bigint mod_divide(const bigint& other) {
      return _div_op(other);
    }

  public:
    inline bigint& operator+=(const bigint& other) {
      if (other._sign == _sign)
        _add_op(other);
      else
        _sub_op(other);

      return *this;
    }
    inline bigint& operator-=(const bigint& other) {
      if (other._sign != _sign)
        _add_op(other);
      else
        _sub_op(other);

      return *this;
    }
    inline bigint& operator*=(const bigint& other) {
      _mul_op(other);
      return *this;
    }
    inline bigint& operator/=(const bigint& other) {
      _div_op(other);
      return *this;
    }

    inline bigint operator+(const bigint& other) const {
      bigint clone = *this;
      clone += other;
      return clone;
    }
    inline bigint operator-(bigint other) const {
      bigint clone = *this;
      clone -= other;
      return clone;
    }
    inline bigint operator*(bigint other) const {
      bigint clone = *this;
      clone *= other;
      return clone;
    }
    inline bigint operator/(bigint other) const {
      bigint clone = *this;
      clone /= other;
      return clone;
    }

  public:
    constexpr bool operator<(const bigint& other) const {
      if (_data.size() != other._data.size())
        return _data.size() < other._data.size();
      else {
        for (auto a = _data.rbegin(), b = other._data.rbegin(); a != _data.rend(); ++a, ++b) {
          if (a == b) continue;
          else return a < b;
        }
      }
      return false;
    }
    constexpr bool operator>(const bigint& other) const {
      if (_data.size() != other._data.size())
        return _data.size() > other._data.size();
      else {
        for (auto a = _data.rbegin(), b = other._data.rbegin(); a < _data.rend(); ++a, ++b) {
          if (a == b) continue;
          else return a > b;
        }
      }
      return false;
    }
    constexpr bool operator>=(const bigint& other) const { return !(*this < other); }
    constexpr bool operator<=(const bigint& other) const { return !(*this > other); }
    inline bool operator==(const bigint& other) const { return _data == other._data; }

  public:
    /// Base MUST be <= 38, or you will incur UB
    std::string to_string(int base = 10) const {
      static const char digits[37] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
      auto cpy = *this;
      std::string ret;

      bigint current_base = base;

      while (!cpy.is_zero()) {
        ret.push_back(digits[static_cast<int>(cpy.mod_divide(current_base))]);
        current_base += base;
      }

      return {ret.rbegin(), ret.rend()};
    }

  public:
    template<typename T, typename = typename std::is_integral<T>::type>
    inline explicit operator T() const {
      if (!can_convert<T>())
        throw std::runtime_error("Cannot fit bigint in requested type");

      nu::data be_data(serialised_size<T>());
      std::copy(_data.rbegin(), _data.rend(),
                be_data.begin() + (serialised_size<T>() - _data.size()));
      auto ret = deserialise<T>(be_data);
      if (!_sign)
        ret = -ret;

      return ret;
    }

    inline explicit operator std::string() const {
      return to_string(10);
    }

  public:
    template<typename T, typename = typename std::enable_if<std::is_integral_v<T>>::type>
    inline bigint(T t) : _data(serialised_size<T>()) {
      if (t < 0) {
        _sign = false;
        t = -t;
      }
      else
        _sign = true;

      nu::data be_data = serialise(t);
      std::copy(be_data.rbegin(), be_data.rend(), _data.begin());
      clean();
    }
  };

  template<>
  constexpr bool bigint::can_convert<float>() const noexcept {
    return true;
  }
  template<>
  constexpr bool bigint::can_convert<double>() const noexcept {
    return true;
  }
}
