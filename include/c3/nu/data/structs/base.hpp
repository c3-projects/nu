#pragma once

#include <string>
#include <memory>
#include <any>
#include <map>
#include <variant>

#include "c3/nu/data.hpp"
#include "c3/nu/bigint.hpp"

namespace c3::nu {
  template<typename StructType>
  using struct_value_t = std::variant<std::monostate, nu::bigint, std::string, std::vector<StructType>>;

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
    inline obj_struct& get_or_add_child(std::string&& name) {
      return get_impl<parent_t>().emplace(name, obj_struct()).first->second;
    }
    inline bool remove_child(std::string&& name) {
      return get_impl<parent_t>().erase(name) != 0;
    }
    inline obj_struct& get_child(std::string&& name) const {
      return std::visit([&](auto& x) -> obj_struct& {
        // If it is not a parent, then there is no chance of finding it,
        // even if it is monostate
        if constexpr (std::is_same_v<decltype(x), parent_t>)
          return x.at(name);
        else throw std::out_of_range("get_child called on incorrect node type");
      }, _impl);
    }
    inline bool rename_child(std::string&& old_name, std::string&& new_name) {
      parent_t& m = get_impl<parent_t>();

      auto n = m.extract(old_name);
      n.key() = new_name;
      m.insert(std::move(n));

      return true;
    }
    inline obj_struct steal_child(std::string&& name) {
      return get_impl<parent_t>().extract(name).mapped();
    }
    inline obj_struct& emplace_child(std::string&& name, obj_struct&& child) {
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
    inline void set_value() {
      get_impl<value_t>() = value_t();
    }
    inline void set_value(std::string str) {
      get_impl<value_t>() = std::move(str);
    }
    inline void set_value(const char* cstr) {
      set_value(std::string{cstr});
    }
    inline void set_value(bigint a) {
      get_impl<value_t>() = a;
    }
    inline void set_value(std::vector<obj_struct> ds) {
      get_impl<value_t>() = ds;
    }
    template<typename Iter>
    inline void set_value(Iter begin, Iter end) {
      get_impl<value_t>() = std::vector<obj_struct>(begin, end);
    }

  public:
    obj_struct& operator[](std::string&& name) { return get_or_add_child(std::move(name)); }

  public:
    template<typename T>
    obj_struct& operator=(T&& t) { set_value(std::forward<T&&>(t)); return *this; }
  };

  template<>
  obj_struct& obj_struct::operator=(obj_struct&& other) {
    _impl = std::move(other._impl);
    return *this;
  };
  template<>
  obj_struct& obj_struct::operator=(const obj_struct& other) {
    _impl = other._impl;
    return *this;
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
    inline markup_struct& add_or_get_child(std::string&& name) {
      return get_impl<parent_t>().emplace(name, obj_struct())->second;
    }
    inline size_t remove_children(std::string&& name) {
      return get_impl<parent_t>().erase(name);
    }
    inline std::pair<parent_t::iterator, parent_t::iterator> get_children(std::string&& name) {
      return std::visit([&](auto& x) -> std::pair<parent_t::iterator, parent_t::iterator> {
        // If it is not a parent, then there is no chance of finding it,
        // even if it is monostate
        if constexpr (std::is_same_v<decltype(x), parent_t>)
          return x.equal_range(name);
        else throw std::out_of_range("get_child called on incorrect node type");
      }, _impl);
    }
    inline std::pair<parent_t::const_iterator, parent_t::const_iterator> get_children(std::string&& name) const {
      return std::visit([&](auto& x) -> std::pair<parent_t::const_iterator, parent_t::const_iterator> {
        // If it is not a parent, then there is no chance of finding it,
        // even if it is monostate
        if constexpr (std::is_same_v<decltype(x), parent_t>)
          return x.equal_range(name);
        else throw std::out_of_range("get_child called on incorrect node type");
      }, _impl);
    }
    inline markup_struct steal_first_child(std::string&& name) {
      return get_impl<parent_t>().extract(name).mapped();
    }
    inline markup_struct& emplace_child(std::string&& name, markup_struct&& child) {
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
    inline void set_value() {
      get_impl<value_t>() = value_t();
    }
    inline void set_value(std::string str) {
      get_impl<value_t>() = std::move(str);
    }
    inline void set_value(const char* cstr) {
      set_value(std::string{cstr});
    }
    inline void set_value(bigint a) {
      get_impl<value_t>() = a;
    }
    inline void set_value(std::vector<markup_struct> ds) {
      get_impl<value_t>() = ds;
    }
    template<typename Iter>
    inline void set_value(Iter begin, Iter end) {
      get_impl<value_t>() = std::vector<markup_struct>(begin, end);
    }

  public:
    template<typename T>
    markup_struct& operator=(T&& t) { set_value(std::forward<T&&>(t)); return *this; }

  public:
    inline std::string& get_or_add_attr(std::string&& name) {
      return _attr.emplace(name, markup_struct()).first->second;
    }
    inline bool remove_attr(std::string&& name) {
      return _attr.erase(name) != 0;
    }
    inline std::string& get_child(std::string&& name) const {
      return std::visit([&](auto& x) -> std::string& {
        // If it is not a parent, then there is no chance of finding it,
        // even if it is monostate
        if constexpr (std::is_same_v<decltype(x), parent_t>)
          return x.at(name);
        else throw std::out_of_range("get_child called on incorrect node type");
      }, _impl);
    }

  public:
    std::string& operator[](std::string&& name) { return get_or_add_attr(std::move(name)); }
  };

  template<>
  markup_struct& markup_struct::operator=(markup_struct&& other) {
    _impl = std::move(other._impl);
    _attr = std::move(other._attr);
    return *this;
  };
  template<>
  markup_struct& markup_struct::operator=(const markup_struct& other) {
    _impl = other._impl;
    _attr = other._attr;
    return *this;
  };
}
