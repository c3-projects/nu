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
    inline obj_struct& operator[](const std::string& name) { return get_or_add_child(std::move(name)); }

  public:
    template<typename T>
    inline void set(T&& t) {
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
    static inline obj_struct empty_parent() {
      obj_struct ret;
      ret._impl.emplace<parent_t>();
      return ret;
    }

  public:
    inline bool operator==(const obj_struct& obj) const { return _impl == obj._impl; }
    inline bool operator!=(const obj_struct& obj) const { return _impl != obj._impl; }
  };

  class markup_struct {
  public:
    using value_t = std::variant<std::string, markup_struct>;

  public:
    class value_tag_t {};
    class elem_tag_t {};
    class attr_tag_t {};

    constexpr static value_tag_t value;
    constexpr static elem_tag_t elem;
    constexpr static attr_tag_t attr;

  public:
    std::string type;
    std::map<std::string, std::string, std::less<>> attrs;
  private:
    std::vector<std::unique_ptr<value_t>> _children;

  public:
    inline std::string& get_or_create_attr(std::string_view attr) {
      // This will emplace if it doesn't exist, and find if it does
      return attrs.emplace(attr, std::string()).first->second;
    }
    inline std::string& get_or_create_attr(std::string&& attr) {
      // This will emplace if it doesn't exist, and find if it does
      return attrs.emplace(std::move(attr), std::string()).first->second;
    }
    inline bool attr_equals(std::string_view attr, std::string_view val) const {
      if (auto iter = attrs.find(attr); iter != attrs.end()) return iter->second == val;
      else return false;
    }

  public:
    template<typename... Args>
    inline std::string& add_value(Args... args) {
      auto& ret = _children.emplace_back(std::make_unique<value_t>(std::in_place_type<std::string>, std::forward<Args>(args)...));
      return std::get<std::string>(*ret);
    }
    template<typename... Args>
    inline markup_struct& add_elem(Args... args) {
      auto& ret = _children.emplace_back(std::make_unique<value_t>(std::in_place_type<markup_struct>, std::forward<Args>(args)...));
      return std::get<markup_struct>(*ret);
    }
    inline void add() {}
    template<typename ConstructorArg, typename... Args>
    inline void add(value_tag_t, ConstructorArg carg, Args... args) {
      add_value(std::forward<ConstructorArg>(carg));
      add(std::forward<Args>(args)...);
    }
    template<typename ConstructorArg, typename... Args>
    inline void add(elem_tag_t, ConstructorArg carg, Args... args) {
      add_elem(std::forward<ConstructorArg>(carg));
      add(std::forward<Args>(args)...);
    }
    template<typename... Args>
    inline void add(attr_tag_t, std::string name, std::string value, Args... args) {
      get_or_create_attr(name) = value;
      add(std::forward<Args>(args)...);
    }
    template<typename... Args>
    inline void add(std::string str, Args... args) {
      _children.emplace_back(std::make_unique<value_t>(std::in_place_type<std::string>, std::move(str)));
      add(std::forward<Args>(args)...);
    }
    template<typename... Args>
    inline void add(markup_struct ms, Args... args) {
      _children.emplace_back(std::make_unique<value_t>(std::in_place_type<markup_struct>, std::move(ms)));
      add(std::forward<Args>(args)...);
    }
    template<typename... Args>
    inline void add(value_t v, Args... args) {
      _children.emplace_back(std::make_unique<value_t>(std::move(v)));
      add(std::forward<Args>(args)...);
    }
    inline markup_struct& get_child_by_attr(std::string_view attr, std::string_view val) const {
      for (auto& i : _children) {
        try {
          if (auto& x = std::get<markup_struct>(*i); x.attr_equals(attr, val))
            return x;
        }
        catch (...) {}
      }
      throw std::out_of_range("No child had the given attribute");
    }
    inline markup_struct& get_child_by_type(std::string_view type) const {
      for (auto& i : _children) {
        try {
          if (auto& x = std::get<markup_struct>(*i); x.type == type)
            return x;
        }
        catch (...) {}
      }
      throw std::out_of_range("No child had the given attribute");
    }
    size_t n_children() const { return _children.size(); }

  public:
    class iterator {
      friend markup_struct;
    private:
      decltype(_children)::iterator iter;

    public:
      using iterator_category = std::random_access_iterator_tag;
      using value_type = value_t;
      using difference_type = decltype(iter)::difference_type;
      using pointer = value_type*;
      using reference = value_type&;

    public:
      inline iterator& operator++() { iter++; return *this; }
      inline iterator& operator--() { iter--; return *this; }
      inline iterator& operator++(int) { ++iter; return *this; }
      inline iterator& operator--(int) { --iter; return *this; }
      inline ssize_t operator-(const iterator& other) { return iter - other.iter; }
      inline iterator operator+(difference_type s) const { return iter + s; }
      inline iterator operator[](difference_type s) const { return iter + s; }
      inline iterator operator-(difference_type s) const { return iter - s; }
      inline iterator& operator+=(difference_type s) { iter += s; return *this; }
      inline iterator& operator-=(difference_type s) { iter -= s; return *this; }
      inline reference operator*() { return **iter; }
      inline pointer operator->() { return (*iter).get(); }
      inline const value_t& operator*() const { return **iter; }
      inline const value_t* operator->() const { return (*iter).get(); }
      inline bool operator==(const iterator& other) const { return iter == other.iter; }
      inline bool operator!=(const iterator& other) const { return iter != other.iter; }

    private:
      inline iterator(decltype(iter)&& iter) : iter{std::move(iter)} {}
    public:
      inline iterator() = default;
    };
    class const_iterator {
      friend markup_struct;
    private:
      decltype(_children)::const_iterator iter;

    public:
      using iterator_category = std::random_access_iterator_tag;
      using value_type = value_t;
      using difference_type = decltype(iter)::difference_type;
      using pointer = const value_type*;
      using reference = const value_type&;

    public:
      inline const_iterator& operator++() { iter++; return *this; }
      inline const_iterator& operator--() { iter--; return *this; }
      inline const_iterator& operator++(int) { ++iter; return *this; }
      inline const_iterator& operator--(int) { --iter; return *this; }
      inline ssize_t operator-(const const_iterator& other) { return iter - other.iter; }
      inline const_iterator operator+(ssize_t s) const { return iter + s; }
      inline const_iterator operator-(ssize_t s) const { return iter - s; }
      inline const_iterator& operator+=(ssize_t s) { iter += s; return *this; }
      inline const_iterator& operator-=(ssize_t s) { iter -= s; return *this; }
      inline reference operator*() const { return **iter; }
      inline pointer operator->() const { return (*iter).get(); }
      inline bool operator==(const const_iterator& other) const { return iter == other.iter; }
      inline bool operator!=(const const_iterator& other) const { return iter != other.iter; }

    private:
      inline const_iterator(decltype(iter)&& iter) : iter{std::move(iter)} {}
    public:
      inline const_iterator() = default;
    };

  public:
    inline iterator begin() { return _children.begin(); }
    inline iterator end() { return _children.end(); }
    inline const_iterator begin() const { return _children.begin(); }
    inline const_iterator end() const { return _children.end(); }
    inline const_iterator cbegin() const { return _children.cbegin(); }
    inline const_iterator cend() const { return _children.cend(); }

  public:
    inline markup_struct(std::string type) : type{std::move(type)} {}
    template<typename... Args>
    inline markup_struct(std::string type, Args... args) : type{std::move(type)} {
      add(std::forward<Args>(args)...);
    }

  public:
    inline markup_struct() = default;
    inline markup_struct(const markup_struct& other) : type{other.type}, attrs{other.attrs} {
      abort();
      for (auto& i : other._children) {
        _children.push_back(std::make_unique<value_t>(*i));
      }
    };
    inline markup_struct(markup_struct&& other) = default;

    inline markup_struct& operator=(const markup_struct& other) {
      abort();
      type = other.type;
      attrs = other.attrs;
      for (auto& i : other._children) {
        _children.push_back(std::make_unique<value_t>(*i));
      }
      return *this;
    }
    inline markup_struct& operator=(markup_struct&& other) = default;

  public:
    inline bool operator==(const markup_struct& other) const {
      return std::equal(begin(), end(), other.begin(), other.end()) && attrs == other.attrs && type == other.type;
    }
    inline bool operator!=(const markup_struct& other) const {
      return !(*this == other);
    }
  };
}


