#pragma once

#include <chrono>
#include <stdexcept>

namespace c3::nu {
  using timeout_t = std::chrono::duration<uint64_t, std::chrono::microseconds::period>;

  using timeout_clock_t = std::chrono::high_resolution_clock;
  using now_t = std::chrono::time_point<timeout_clock_t, timeout_t>;

  inline now_t now() {
    return std::chrono::time_point_cast<timeout_t>(timeout_clock_t::now());
  }

  class timed_out : public std::exception {};
}
