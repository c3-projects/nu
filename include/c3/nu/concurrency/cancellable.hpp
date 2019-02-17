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
    PartiallyProvided,
    Cancelled,
    Undecided,
    AlreadyDecided
  };

  template<typename T>
  class cancellable_provider;

  template<typename T, typename Base>
  class mapped_cancellable;

  /// Taking the result of a completed cancellable is not thread safe
  template<typename T>
  class cancellable {
    template<typename All>
    friend class cancellable_provider;

    template<typename All>
    friend class cancellable;
  public:
    using provided_t = T;

  private:
    class shared_state_t {
    public:
      virtual gateway_bool& final_state_decided() = 0;
      virtual const gateway_bool& final_state_decided() const = 0;
      virtual gateway_bool& some_state_decided() = 0;
      virtual const gateway_bool& some_state_decided() const = 0;
      virtual std::optional<T> take_value() = 0;
      virtual void set_value(T&&) = 0;
      virtual bool has_value() const = 0;

    public:
      inline bool is_cancelled() const {
        return final_state_decided() && !has_value();
      }

      inline cancellable_state get_state() const {
        if (!final_state_decided()) {
          if (some_state_decided())
            return cancellable_state::PartiallyProvided;
          else
            return cancellable_state::Undecided;
        }
        if (has_value()) return cancellable_state::Provided;
        else return cancellable_state::Cancelled;
      }
    };
    class simple_state;
    template<typename Base>
    class mapped_state;
    class predetermined_state;

  protected:
    std::shared_ptr<shared_state_t> shared_state;

  public:
    /// Blocks until a result is provided or cancelled
    inline void wait() {
      shared_state->some_state_decided().wait_for_open();
    }
    /// Blocks until a result is provided or cancelled, or the timeout is reached
    inline bool wait(timeout_t timeout) {
      return shared_state->some_state_decided().wait_for_open(timeout);
    }

    /// Blocks until a final result is provided or cancelled
    inline void wait_final() {
      shared_state->final_state_decided().wait_for_open();
    }
    /// Blocks until a final result is provided or cancelled, or the timeout is reached
    inline bool wait_final(timeout_t timeout) {
      return shared_state->final_state_decided().wait_for_open(timeout);
    }

    inline void finalise() {
      // Cancels if not set, finalises existing state
      shared_state->final_state_decided().maybe_open([&] {
        shared_state->some_state_decided().open();
        return true;
      });
    }

    inline bool finalise(timeout_t timeout) {
      // Since this is lockless, we cannot reliable report whether the final data was a final provision
      // Wait for the final result
      auto ret = shared_state->final_state_decided().open_after(timeout);
      // This doesn't need to be locked,
      // as all functions should check final_state_decided first
      shared_state->some_state_decided().open();
      return ret;
    }

    inline bool is_cancelled() { return shared_state->is_cancelled(); }

    inline bool is_decided() { return shared_state->final_state_decided(); }

    inline cancellable_state get_state() const { return shared_state->get_state(); }

    /// Takes the result if it has been provided, otherwise returns std::nullopt
    inline std::optional<T> try_take() {
      if (shared_state->final_state_decided())
        return shared_state->take_value();
      else if (shared_state->some_state_decided()) {
        shared_state->final_state_decided().open();
        return shared_state->take_value();
      }
      else
        return std::nullopt;
    }

    /// Takes the final result if it has been provided, otherwise returns std::nullopt
    inline std::optional<T> try_take_final() {
      if (shared_state->final_state_decided())
        return shared_state->take_value();
    }

    /// Returns the result if it has been provided, otherwise returns std::nullopt
    inline std::optional<T> try_get(timeout_t timeout) {
      wait(timeout);
      return try_get();
    }

    /// Takes the result if it has been provided, otherwise returns std::nullopt
    inline std::optional<T> try_take(timeout_t timeout) {
      wait(timeout);
      return try_take();
    }

    /// Returns the final result if it has been provided, otherwise returns std::nullopt
    inline std::optional<T> try_get_final(timeout_t timeout) {
      wait_final(timeout);
      return try_take();
    }

    /// Takes the final result if it has been provided, otherwise returns std::nullopt
    inline std::optional<T> try_take_final(timeout_t timeout) {
      wait_final(timeout);
      return try_take();
    }

    template<typename Ret>
    inline cancellable<Ret> map(std::function<Ret(T)> func) {
      return {std::make_shared<mapped_state<Ret>>(shared_state, std::move(func))};
    }

    inline void get_on_complete(std::function<void(T)> func) {
      std::thread([=, func{std::move(func)}]() {
        wait_final();
        if (auto x = try_get())
          func(std::move(*x));
      }).detach();
    }

    inline void take_on_complete(std::function<void(T)> func) {
      std::thread([=, func{std::move(func)}]() {
        wait_final();
        if (auto x = try_take())
          func(std::move(*x));
      }).detach();
    }

  public:
    template<typename Other>
    operator cancellable<Other>() { return map([](T t) { return Other{t}; }); }

  public:
    // For when you need to return a cancellable, but you already have the value
    inline cancellable(T t) : shared_state{std::make_shared<simple_state>()} {
      shared_state->set_value(std::move(t));
      shared_state->final_state_decided().open();
      shared_state->some_state_decided().open();
    }

  private:
    inline cancellable(decltype(shared_state) shared_state) : shared_state{shared_state} {}
  };

  template<typename T>
  class cancellable<T>::simple_state final : public cancellable<T>::shared_state_t {
  private:
    std::optional<T> result = std::nullopt;

    gateway_bool _some_state_decided;
    gateway_bool _final_state_decided;

  public:
    inline gateway_bool& final_state_decided() override { return _final_state_decided; }
    inline const gateway_bool& final_state_decided() const override { return _final_state_decided; }
    inline const gateway_bool& some_state_decided() const override { return _some_state_decided; }
    inline gateway_bool& some_state_decided() override { return _some_state_decided; }
    inline std::optional<T> take_value() override {
      if (result) {
        std::optional<T> ret = std::move(*result);
        result.reset();
        return ret;
      }
      return {};
    }
    inline void modify_value(std::function<void(std::optional<T>&)> func) {
      func(result);
    }
    inline void set_value(T&& value) override {
      result = std::move(value);
    }
    inline bool has_value() const override { return result.has_value(); }
  };

  /// Acts both as a mapper for set
  template<typename Base>
  template<typename T>
  class cancellable<Base>::mapped_state final : public cancellable<T>::shared_state_t {
  private:
    std::shared_ptr<typename cancellable<Base>::shared_state_t> base;
    std::function<T(Base)> mapper;

  public:
    inline gateway_bool& final_state_decided() override { return base->final_state_decided(); }
    inline gateway_bool& some_state_decided() override { return base->some_state_decided(); }
    inline const gateway_bool& final_state_decided() const override { return base->final_state_decided(); }
    inline const gateway_bool& some_state_decided() const override { return base->some_state_decided(); }
    inline std::optional<T> take_value() override {
      if (auto value = base->take_value())
        return { mapper(*std::move(value)) };
      else
        return std::nullopt;
    }
    inline void set_value(T&&) override {
      throw std::logic_error("cancellable<T>::mapped_state used to set value");
    }
    inline bool has_value() const override { return base->has_value(); }

  public:
    mapped_state(decltype(base) base, decltype(mapper) mapper) : base{base}, mapper{mapper} {}
  };

  template<typename T>
  class cancellable_provider {
  public:
    using provided_t = T;

  private:
    template<typename Base>
    class mapped_state;

  private:
    std::shared_ptr<typename cancellable<T>::simple_state> shared_state;

  public:
    /// Attempts to provide a value, but does not call func if a result is already set
    ///
    /// If the function throws an exception, the result is cancelled
    inline cancellable_state maybe_provide(std::function<std::optional<T>()> func) {
      cancellable_state ret;

      shared_state->final_state_decided().maybe_open([&] {
        try {
          if (auto result = func()) {
            shared_state->set_value(std::move(*result));
            shared_state->some_state_decided().open();
            ret = cancellable_state::Provided;

            return true;
          }
          else {
            ret = cancellable_state::Undecided;
            return false;
          }
        } catch(...) {
          ret = cancellable_state::Cancelled;
          shared_state->some_state_decided().open();
          return true;
        }
      },
      [&] {
        ret = cancellable_state::AlreadyDecided;
      });

      return ret;
    }

    /// Similar to maybe_provide, but does not set the final state unless cancelled.
    ///
    /// If the function throws an exception, the result is cancelled
    inline cancellable_state maybe_update(std::function<std::optional<T>()> func) {
      cancellable_state ret;

      shared_state->final_state_decided().maybe_open([&] {
        try {
          if (auto result = func()) {
            shared_state->set_value(std::move(*result));
            shared_state->some_state_decided().open();
            ret = cancellable_state::PartiallyProvided;

            return false;
          }
          else {
            ret = cancellable_state::Undecided;
            return false;
          }
        } catch(...) {
          ret = cancellable_state::Cancelled;
          shared_state->some_state_decided().open();
          return true;
        }
      },
      [&] {
        ret = cancellable_state::AlreadyDecided;
      });

      return ret;
    }

    /// Similar to maybe_update, but func is passed the shared state
    ///
    /// If the function throws an exception, the result is cancelled
    ///
    /// Please note that for mapped cancellables, the value is taken and then replaced,
    /// resulting in 2 copies
    inline cancellable_state maybe_modify(std::function<void(std::optional<T>&)> func) {
      cancellable_state ret;

      shared_state->final_state_decided().maybe_open([&] {
        try {
          shared_state->modify_value(std::move(func));

          if (shared_state->has_value()) {
            shared_state->some_state_decided().open();
            ret = cancellable_state::PartiallyProvided;

            return false;
          }
          else {
            ret = cancellable_state::Undecided;
            return false;
          }
        } catch(...) {
          ret = cancellable_state::Cancelled;
          shared_state->some_state_decided().open();
          return true;
        }
      },
      [&] {
        ret = cancellable_state::AlreadyDecided;
      });

      return ret;
    }

    // Returns true if the value was undecided before the function was called
    inline bool provide(T val) {
      bool ret;

      shared_state->final_state_decided().maybe_open([&] {
        try {
          shared_state->set_value(std::move(val));
          shared_state->some_state_decided().open();
          ret = true;
        } catch(...) {
          ret = true;
          shared_state->some_state_decided().open();
        }
        return true;
      },
      [&] {
        ret = false;
      });

      return ret;
    }

    // Returns true if the value was undecided before the function was called
    inline bool update(T val) {
      bool ret;

      shared_state->final_state_decided().maybe_open([&] {
        ret = true;
        try {
          shared_state->set_value(std::move(val));
        } catch(...) {}
        shared_state->some_state_decided().open();
        return false;
      },
      [&] {
        ret = false;
      });

      return ret;
    }

    inline bool is_cancelled() { return shared_state->is_cancelled(); }

    inline bool is_decided() { return shared_state->final_state_decided(); }

    inline cancellable_state get_state() const { return shared_state->get_state(); }

    inline void cancel() { shared_state->final_state_decided().open(); }

  public:
    inline cancellable<T> get_cancellable() { return { shared_state }; }

  public:
    cancellable_provider() : shared_state{std::make_shared<typename cancellable<T>::simple_state>()} {}
    cancellable_provider(decltype(shared_state) shared_state) : shared_state{shared_state} {}
  };
}
