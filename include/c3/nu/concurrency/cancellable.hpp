#pragma once

#include <optional>
#include <mutex>
#include <atomic>
#include <memory>
#include <functional>
#include <condition_variable>
#include <future>

#include "c3/nu/concurrency/timeout.hpp"
#include "c3/nu/concurrency/mutexed.hpp"
#include "c3/nu/concurrency/gateway.hpp"

//! A collection of objects similar to std::future, but with the ability to cancel the result in
//! a safe manner
namespace c3::nu {
  /// An enum describing the current state of a cancellable provider
  /// and all of its cancellable chidlren
  enum class cancellable_state {
    Provided,
    Cancelled,
    Empty
  };

  template<typename T>
  class cancellable_provider;

  template<typename T>
  class cancellable {
    friend cancellable_provider<T>;

  private:
    class shared_state_t {
    public:
      std::optional<T> result = std::nullopt;

      gateway_bool final_state_decided;

    public:
      bool is_cancelled() const {
        return final_state_decided && !result.has_value();
      }

      inline cancellable_state get_state() const {
        if (!final_state_decided) return cancellable_state::Empty;
        if (result) return cancellable_state::Provided;
        else return cancellable_state::Cancelled;
      }
    };

  private:
    std::shared_ptr<shared_state_t> shared_state;

  public:
    /// Returns the result if it has been provided or cancelled, otherwise cancels the result
    std::optional<T> get_or_cancel() {
      // Cancel if not set
      shared_state->final_state_decided.maybe_open([&] { return true; });

      return shared_state->result;
    }

    /// Returns the result if it has been provided, (or std::nullopt if it was cancelled)
    /// within the given timeout, otherwise cancels the result
    std::optional<T> get_or_cancel(timeout_t timeout) {
      shared_state->final_state_decided.open_after(timeout);
      return shared_state->result;
    }

    inline bool is_cancelled() { return shared_state->is_cancelled(); }

    /// Returns the result if it has been provided, otherwise returns std::nullopt
    inline std::optional<T> try_get() {
      if (shared_state->final_state_decided)
        return shared_state->result;
      else
        return std::nullopt;
    }

    inline cancellable_state get_state() const { return shared_state->get_state(); }

  private:
    inline cancellable(decltype(shared_state) shared_state) : shared_state{shared_state} {}
  };

  template<typename T>
  class cancellable_provider {
  private:
    std::shared_ptr<typename cancellable<T>::shared_state_t> shared_state =
      std::make_shared<typename cancellable<T>::shared_state_t>();

  public:
    /// Blocks all requests for state, and if func returns a std::optional with a value sets the result.
    ///
    /// If the function throws an exception, the result is cancelled
    template<typename Func>
    inline cancellable_state maybe_provide(Func func) {
      cancellable_state ret;

      shared_state->final_state_decided.maybe_open([&] {
        try {
          auto result = func();

          if (result) {
            shared_state->result = std::move(result);
            ret = cancellable_state::Provided;

            return true;
          }
          else {
            ret = cancellable_state::Empty;
            return false;
          }
        } catch(...) {
          ret = cancellable_state::Cancelled;
          return true;
        }
      });

      return ret;
    }

    inline bool is_cancelled() { return shared_state->is_cancelled(); }

    inline cancellable_state get_state() const { return shared_state->get_state(); }

    inline void cancel() { shared_state->final_state_decided.open(); }

  public:
    inline cancellable<T> get_cancellable() { return { shared_state }; }
  };
}
