#pragma once

#include <string>
#include <memory>
#include <any>
#include <map>
#include <variant>

#include "c3/nu/data.hpp"

namespace c3::nu {
  //TODO iterator
  class data_struct {
  public:
    virtual data_struct& add_or_get_child(std::string&& name) = 0;
    virtual data_struct& add_or_insert_child(std::string&& name) = 0;
    virtual bool remove_child(std::string&& name) = 0;
    /// Returns nullptr on error
    virtual data_struct* get_child(std::string&& name) noexcept = 0;
    virtual bool rename_child(std::string&& old_name, std::string&& new_name) = 0;

  public:
    virtual std::any& get_value() = 0;
    virtual void set_value(std::any&&) = 0;
    /// Returns false if old_name does not refer to an existing child

  public:
    virtual bool is_value() const;
    virtual bool is_parent() const;

  public:
    template<typename T>
    data_struct& operator=(T&& t) {
      if (is_parent()) {
        // somehow take children with iter
        throw std::runtime_error("Parent child steal not implemented");
        return *this;
      }
      else {
        return set_value(t);
      }
    }

  public:
    data_struct& operator[](std::string&& name) { return add_or_get_child(std::move(name)); }

  public:
    virtual ~data_struct();
  };

  class simple_data_struct final : public data_struct {
  private:
    using map_t = std::map<std::string&&, simple_data_struct>;

    std::variant<void, std::any, map_t> _impl;

  public:
    inline data_struct& add_or_get_child(std::string&& name) override {
      return std::get<map_t>(_impl)[name] = simple_data_struct();
    }
    inline data_struct& add_or_insert_child(std::string&& name) override {
      return std::get<map_t>(_impl).insert_or_assign(name, simple_data_struct()).first->second;
    }
    inline bool remove_child(std::string&& name) override {
      return std::get<map_t>(_impl).erase(name) != 0;
    }
    inline data_struct* get_child(std::string&& name) noexcept override {
      try {
        return &std::get<map_t>(_impl).at(name);
      }
      catch (...) {}

      return nullptr;
    }
    inline bool rename_child(std::string&& old_name, std::string&& new_name) override {}

  public:
    inline std::any& get_value() override {
      return std::get<std::any>(_impl);
    }
    inline void set_value(std::any&&) override {
      return std::get<std::any>(_impl);
    }

  public:
    inline bool is_value() const override {}
    inline bool is_parent() const override {}
  };
}
