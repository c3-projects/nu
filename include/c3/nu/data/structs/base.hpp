#pragma once

#include <string>
#include <memory>
#include <any>
#include <map>
#include <variant>

#include "c3/nu/data.hpp"

namespace c3::nu {
  class obj_struct {
  private:
    using parent_t = std::map<std::string, obj_struct>;
  public:
    using int_t = int64_t;
    using float_t = double;
    using string_t = std::string;
    using bool_t = bool;
    using arr_t = std::vector<obj_struct>;
    using value_t = std::variant<std::monostate,
                                 int_t,
                                 float_t,
                                 string_t,
                                 bool_t,
                                 arr_t>;

  private:
    std::variant<std::monostate, parent_t, value_t> _impl;

  public:
    inline bool can_be_value() const {
      return !std::holds_alternative<parent_t>(_impl);
    }
    inline bool can_be_parent() const {
      return !std::holds_alternative<value_t>(_impl);
    }
    inline bool is_value() const {
      return std::holds_alternative<value_t>(_impl);
    }
    inline bool is_parent() const {
      return std::holds_alternative<parent_t>(_impl);
    }

  private:
    template<typename T>
    T& get_impl() {
      return get_or_create_alternative<T>(_impl);
    }

  public:
    inline obj_struct& get_or_add_child(const std::string& name) {
      return get_impl<parent_t>().emplace(name, obj_struct()).first->second;
    }
    inline bool remove_child(const std::string& name) {
      return get_impl<parent_t>().erase(name) != 0;
    }
    inline obj_struct& get_child(const std::string& name) const {
      return std::visit([&](auto& x) -> obj_struct& {
        // If it is not a parent, then there is no chance of finding it,
        // even if it is monostate
        if constexpr (std::is_same_v<decltype(x), parent_t>)
          return x.at(name);
        else throw std::out_of_range("get_child called on incorrect node type");
      }, _impl);
    }
    inline bool rename_child(const std::string& old_name, const std::string& new_name) {
      parent_t& m = get_impl<parent_t>();

      auto n = m.extract(old_name);
      n.key() = new_name;
      m.insert(std::move(n));

      return true;
    }
    inline obj_struct steal_child(const std::string& name) {
      return get_impl<parent_t>().extract(name).mapped();
    }
    inline obj_struct& emplace_child(const std::string& name, obj_struct&& child) {
      return get_impl<parent_t>().emplace(name, child).first->second;
    }
    inline parent_t::iterator begin() { return std::get<parent_t>(_impl).begin(); }
    inline parent_t::iterator end() { return std::get<parent_t>(_impl).end(); }
    inline parent_t::const_iterator begin() const { return std::get<parent_t>(_impl).begin(); }
    inline parent_t::const_iterator end() const { return std::get<parent_t>(_impl).end(); }
    inline parent_t::const_iterator cbegin() const { return std::get<parent_t>(_impl).cbegin(); }
    inline parent_t::const_iterator cend() const { return std::get<parent_t>(_impl).cend(); }

  public:
    template<typename T>
    inline T& as() {
      return get_or_create_alternative<T>(get_impl<value_t>());
    }
    template<typename T>
    inline const T& as() const {
      return std::get<T>(std::get<value_t>(_impl));
    }
    inline value_t& get_value() {
      return std::get<value_t>(_impl);
    }
    inline const value_t& get_value() const {
      return std::get<value_t>(_impl);
    }
    template<typename T, typename... Args>
    inline void set_value(Args&&... args) {
      get_impl<value_t>().emplace<T>(std::forward<Args>(args)...);
    }

  public:
    obj_struct& operator[](const std::string& name) { return get_or_add_child(std::move(name)); }

  public:
    template<typename T>
    void set(T&& t) {
      using U = typename remove_all<T>::type;

      if constexpr (std::is_same_v<U, obj_struct>) {
        if constexpr (std::is_lvalue_reference_v<T>)
          _impl = t._impl;
        else
          _impl = std::move(t._impl);
      }
      else if constexpr (std::is_same_v<U, std::nullptr_t> || std::is_same_v<U, std::monostate>)
        set_value<std::monostate>();
      else if constexpr (std::is_same_v<U, bool>)
        set_value<bool>(t);
      else if constexpr (std::is_floating_point_v<U>)
        set_value<double>(t);
      else if constexpr (std::is_integral_v<U>)
        set_value<int64_t>(t);
      else if constexpr (std::is_same_v<U, arr_t>)
        set_value<arr_t>(t);
      else
        set_value<std::string>(std::forward<T&&>(t));
    }

    template<typename T>
    inline obj_struct& operator=(T&& t) {
      set<T>(std::forward<T>(t));
      return *this;
    }

    template<typename T>
    inline obj_struct(T&&t) { set<T>(std::forward<T>(t)); }

    inline obj_struct() = default;

  public:
    static obj_struct empty_parent() {
      obj_struct ret;
      ret._impl.emplace<parent_t>();
      return ret;
    }

  public:
    bool operator==(const obj_struct& obj) const { return _impl == obj._impl; }
    bool operator!=(const obj_struct& obj) const { return _impl != obj._impl; }
  };
}
