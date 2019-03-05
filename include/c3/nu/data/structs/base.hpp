#pragma once

#include <string>
#include <memory>
#include <any>
#include <map>
#include <variant>

#include "c3/nu/data.hpp"

namespace c3::nu {
  template<typename StructType>
  using struct_value_t = std::variant<std::monostate,
                                      int64_t,
                                      double,
                                      std::string,
                                      bool,
                                      std::vector<StructType>>;

  class obj_struct {
  private:
    using parent_t = std::map<std::string, obj_struct>;
  public:
    using value_t = struct_value_t<obj_struct>;

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
      if (std::holds_alternative<std::monostate>(_impl))
        return _impl.emplace<T>();
      else
        return std::get<T>(_impl);
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
    inline value_t& get_or_create_value() {
      return get_impl<value_t>();
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
      else if constexpr (std::is_same_v<U, bool>)
        set_value<bool>(t);
      else if constexpr (std::is_floating_point_v<U>)
        set_value<double>(t);
      else if constexpr (std::is_integral_v<U>)
        set_value<int64_t>(t);
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
    bool operator==(const obj_struct& obj) const { return _impl == obj._impl; }
    bool operator!=(const obj_struct& obj) const { return _impl != obj._impl; }
  };

  class markup_struct {
  private:
    using parent_t = std::multimap<std::string, markup_struct>;
    using attr_t = std::map<std::string, std::string>;
  public:
    using value_t = struct_value_t<markup_struct>;

  private:
    std::variant<std::monostate, parent_t, value_t> _impl;
    attr_t _attr;

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
      if (std::holds_alternative<std::monostate>(_impl))
        return _impl.emplace<T>();
      else
        return std::get<T>(_impl);
    }

  public:
    inline markup_struct& add_or_get_child(const std::string& name) {
      return get_impl<parent_t>().emplace(name, markup_struct())->second;
    }
    inline size_t remove_children(const std::string& name) {
      return get_impl<parent_t>().erase(name);
    }
    inline std::pair<parent_t::iterator, parent_t::iterator> get_children(const std::string& name) {
      return std::visit([&](auto& x) -> std::pair<parent_t::iterator, parent_t::iterator> {
        // If it is not a parent, then there is no chance of finding it,
        // even if it is monostate
        if constexpr (std::is_same_v<decltype(x), parent_t>)
          return x.equal_range(name);
        else throw std::out_of_range("get_child called on incorrect node type");
      }, _impl);
    }
    inline std::pair<parent_t::const_iterator, parent_t::const_iterator> get_children(const std::string& name) const {
      return std::visit([&](auto& x) -> std::pair<parent_t::const_iterator, parent_t::const_iterator> {
        // If it is not a parent, then there is no chance of finding it,
        // even if it is monostate
        if constexpr (std::is_same_v<decltype(x), parent_t>)
          return x.equal_range(name);
        else throw std::out_of_range("get_child called on incorrect node type");
      }, _impl);
    }
    inline markup_struct steal_first_child(const std::string& name) {
      return get_impl<parent_t>().extract(name).mapped();
    }
    inline markup_struct& emplace_child(const std::string& name, markup_struct&& child) {
      return get_impl<parent_t>().emplace(name, child)->second;
    }
    inline parent_t::iterator begin() { return std::get<parent_t>(_impl).begin(); }
    inline parent_t::iterator end() { return std::get<parent_t>(_impl).end(); }
    inline parent_t::const_iterator begin() const { return std::get<parent_t>(_impl).begin(); }
    inline parent_t::const_iterator end() const { return std::get<parent_t>(_impl).end(); }
    inline parent_t::const_iterator cbegin() const { return std::get<parent_t>(_impl).cbegin(); }
    inline parent_t::const_iterator cend() const { return std::get<parent_t>(_impl).cend(); }

  public:
    inline value_t& get_or_create_value() {
      return get_impl<value_t>();
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
    inline std::string& get_or_add_attr(std::string_view name) {
      return _attr.emplace(name, "").first->second;
    }
    inline std::string& set_attr(const std::string& name, std::string value) {
      return _attr.insert_or_assign(name, std::move(value)).first->second;
    }
    inline bool remove_attr(const std::string& name) {
      return _attr.erase(name) != 0;
    }
    inline std::string& get_child(const std::string& name) const {
      return std::visit([&](auto& x) -> std::string& {
        // If it is not a parent, then there is no chance of finding it,
        // even if it is monostate
        if constexpr (std::is_same_v<decltype(x), parent_t>)
          return x.at(name);
        else throw std::out_of_range("get_child called on incorrect node type");
      }, _impl);
    }

  public:
    std::string& operator[](const std::string& name) { return get_or_add_attr(std::move(name)); }

  public:
    template<typename T>
    void set(T&& t) {
      using U = typename remove_all<T>::type;

      if constexpr (std::is_same_v<U, markup_struct>) {
        if constexpr (std::is_lvalue_reference_v<T>) {
          _impl = t._impl;
          _attr = t._attr;
        }
        else {
          _impl = std::move(t._impl);
          _attr = std::move(t._attr);
        }
      }
      else if constexpr (std::is_same_v<U, bool>)
        set_value<bool>(t);
      else if constexpr (std::is_floating_point_v<U>)
        set_value<double>(t);
      else if constexpr (std::is_integral_v<U>)
        set_value<int64_t>(t);
      else
        set_value<std::string>(std::forward<T&&>(t));
    }
    template<typename T>
    inline markup_struct& operator=(T&& t) {
      set<T>(std::forward<T>(t));
      return *this;
    }

    template<typename T>
    inline markup_struct(T&&t) { set<T>(std::forward<T>(t)); }

    inline markup_struct() = default;

  public:
    bool operator==(const markup_struct& obj) const { return _impl == obj._impl && _attr == obj._attr; }
    bool operator!=(const markup_struct& obj) const { return _impl != obj._impl && _attr != obj._attr; }
  };
}
