#pragma once

#include <string>
#include <memory>
#include <any>
#include <map>
#include <variant>

#include "c3/nu/data.hpp"

namespace c3::nu {
  /*
  //TODO iterator
  class data_struct {
  public:
    virtual data_struct& add_or_get_child(std::string&& name) = 0;
    virtual data_struct& add_or_insert_child(std::string&& name) = 0;
    virtual bool remove_child(std::string&& name) = 0;
    /// Throws std::out_of_range if the child is not found
    virtual data_struct& get_child(std::string&& name) = 0;
    /// Returns false if old_name does not refer to an existing child
    ///
    /// XXX: overwrites any child with new_name
    virtual bool rename_child(std::string&& old_name, std::string&& new_name) = 0;

  public:
    virtual std::any& get_value() = 0;
    virtual void set_value(std::any&&) = 0;

  public:
    virtual bool can_be_value() const;
    virtual bool can_be_parent() const;

  public:
    template<typename T>
    data_struct& operator=(T&& t) {
      if (can_be_parent()) {
        // somehow take children with iter
        throw std::runtime_error("Parent child steal not implemented");
        return *this;
      }
      else {
        return set_value(std::move(t));
      }
    }

  public:
    data_struct& operator[](std::string&& name) { return add_or_get_child(std::move(name)); }

  public:
    virtual ~data_struct() = default;
  };

  */
  class data_struct {
  private:
    using parent_t = std::map<std::string, data_struct>;
    using value_t = std::any;

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
      if (std::holds_alternative<std::monostate>(_impl) == 0)
        return _impl.emplace<T>();
      else
        return std::get<T>(_impl);
    }

  public:
    inline data_struct& add_or_get_child(std::string&& name) {
      return get_impl<parent_t>()[name] = data_struct();
    }
    inline data_struct& add_or_insert_child(std::string&& name) {
      return get_impl<parent_t>().insert_or_assign(name, data_struct()).first->second;
    }
    inline bool remove_child(std::string&& name) {
      return get_impl<parent_t>().erase(name) != 0;
    }
    inline data_struct& get_child(std::string&& name) const {
      return std::visit([&](auto& x) -> data_struct& {
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
    inline data_struct steal_child(std::string&& name) {
      return get_impl<parent_t>().extract(name).mapped();
    }
    inline data_struct& emplace_child(std::string&& name, data_struct&& child) {
      return get_impl<parent_t>().emplace(name, child).first->second;
    }
    inline parent_t::iterator begin() { return std::get<parent_t>(_impl).begin(); }
    inline parent_t::iterator end() { return std::get<parent_t>(_impl).end(); }
    inline parent_t::const_iterator begin() const { return std::get<parent_t>(_impl).begin(); }
    inline parent_t::const_iterator end() const { return std::get<parent_t>(_impl).end(); }
    inline parent_t::const_iterator cbegin() const { return std::get<parent_t>(_impl).cbegin(); }
    inline parent_t::const_iterator cend() const { return std::get<parent_t>(_impl).cend(); }

  public:
    inline std::any& get_or_create_value() {
      return get_impl<value_t>();
    }
    inline std::any& get_value() {
      return std::get<value_t>(_impl);
    }
    inline const std::any& get_value() const {
      return std::get<value_t>(_impl);
    }
    inline void set_value(std::any&& a) {
      get_impl<value_t>() = a;
    }

  public:
    data_struct& operator[](std::string&& name) { return add_or_get_child(std::move(name)); }

  public:
    template<typename T>
    inline data_struct& operator=(T&& t) {
      set_value(std::forward<T>(t));
      return *this;
    }
  };

  template<>
  inline data_struct& data_struct::operator=(data_struct&& ds) {
    _impl = std::move(ds._impl);
    return *this;
  }
}
