#pragma once

// Expand cpptoml library

#ifndef __TOMLPP_ORM_HPP__
#define __TOMLPP_ORM_HPP__

#include "tomlpp/toml.hpp"
#include <iostream>
#include <typeinfo>
#include <list>
#include <variant>
#include <functional>
#include <algorithm>
#include <map>
#include <unordered_map>

#define TOML_D(...) __VA_ARGS__, #__VA_ARGS__


namespace toml::orm {
    namespace detail {
        template<typename T, typename U>
        constexpr inline T only_str(U&& type) {
            if constexpr (std::is_same_v<T,std::string>) { return std::string{type};} else {return type;}
        }

        inline void panic(bool opt, const std::string& message) {
            if(!opt) {
                std::cerr << message << std::endl;
                exit(1);
            }
        }
        template<typename T>
        inline auto panic(std::shared_ptr<T> ptr, const std::string& message) {
            if(!ptr) {
                std::cerr << message << std::endl;
                exit(1);
            }
            return ptr;
        }

        inline std::string toml_error_meesage(const std::string& name, const std::string type="table", const std::string parent ="") {
            auto p = (parent != "") ? parent +"." : "";
            return "[toml parse error] Require " + type + ": -> " + p + name;
        }


        template<class T> struct is_vector {
            using type = T;
            using element = T;
            constexpr static bool value = false;
        };

        template<class T>
        struct is_vector<std::vector<T>> {
            using type = std::vector<T>;
            using element = T;
            constexpr  static bool value = true;
        };

        template< typename T> inline constexpr bool is_vector_v = is_vector<T>::value;
        template< typename T> using is_vector_t = typename is_vector<T>::type;
        template< typename T> using is_vector_e = typename is_vector<T>::element;

        template<class T> struct is_list {
            using type = T;
            using element = T;
            constexpr static bool value = false;
        };

        template<class T>
        struct is_list<std::list<T>> {
            using type = std::list<T>;
            using element = T;
            constexpr  static bool value = true;
        };

        template< typename T> inline constexpr bool is_list_v = is_list<T>::value;
        template< typename T> using is_list_t = typename is_list<T>::type;
        template< typename T> using is_list_e = typename is_list<T>::element;

        template<class T> struct is_optional {
            using type = T;
            using element = T;
            constexpr static bool value = false;
        };

        template<class T> struct is_optional<std::optional<T>> {
            using type = std::optional<T>;
            using element = T;
            constexpr static bool value = true;
        };

        template< typename T> inline constexpr bool is_optional_v = is_vector<T>::value;
        template< typename T> using is_optional_t = typename is_optional<T>::type;
        template< typename T> using is_optional_e = typename is_optional<T>::element;


        template<class T>
        struct is_map {
            using type = T;
            using key = T;
            using element = T;
            constexpr static bool value = false;
        };

        template<class T, class U>
        struct is_map<std::map<T,U>> {
            using type = std::map<T,U>;
            using key = T;
            using element = U;
            constexpr static bool value = true;
        };

        template<class T, class U>
        struct is_map<std::unordered_map<T,U>> {
            using type = std::unordered_map<T,U>;
            using key = T;
            using element = U;
            constexpr static bool value = true;
        };

        template< typename T> inline constexpr bool is_map_v = is_map<T>::value;
        template< typename T> using is_map_t = typename is_map<T>::type;
        template< typename T> using is_map_k = typename is_map<T>::key;
        template< typename T> using is_map_e = typename is_map<T>::element;
    }

    using  table_ptr = std::shared_ptr<table>;
    using  base_ptr = std::shared_ptr<base>;
    struct type_interface {
        template<typename T> using opt = std::optional<T>;
        template<typename T> using arr = std::vector<T>;
        template<typename T> using nested = std::map<std::string,T>;
        friend class access;
    };

    struct table : public type_interface {};
    struct inline_define : public type_interface {};

    template<typename T>
    struct is_array {
        constexpr static bool value = detail::is_vector_v<T>;
    };
    template< typename T> inline constexpr bool is_array_v = is_array<T>::value;

    template<typename T>
    struct is_table_array {
        constexpr static bool value = is_array_v<T> && std::is_base_of_v<table, detail::is_vector_e<T>>;
    };
    template< typename T> inline constexpr bool is_table_array_v = is_table_array<T>::value;
    
    template<typename T>
    struct is_table {
        constexpr static bool value = std::is_base_of_v<table, std::decay_t<T>>;
    };
    template< typename T> inline constexpr bool is_table_v = is_table<T>::value;

    template<typename T>
    struct is_inline {
        constexpr static bool value = std::is_base_of_v<inline_define, std::decay_t<T>>;
    };
    template< typename T> inline constexpr bool is_inline_v = is_inline<T>::value;

    template<typename T>
    struct is_nested_table {
        constexpr static bool value = detail::is_map_v<T> && std::is_base_of_v<table, detail::is_map_e<T>>;
    };
    template< typename T> inline constexpr bool is_nested_table_v = is_nested_table<T>::value;

    template<typename T>
    struct is_list_table {
        constexpr static bool value = detail::is_list_v<T> && std::is_base_of_v<table, detail::is_list_e<T>>;
    };
    template< typename T> inline constexpr bool is_list_table_v = is_list_table<T>::value;

    template<typename T> inline table_ptr table_or_value(base_ptr ptr, T& value) {
        if(ptr->is_value()) {
            value = get_impl<T>(ptr);
            return make_table();
        }
        return ptr->as_table();
    }

    template<typename T> inline table_ptr table_or_value(base_ptr ptr, std::optional<T>& value) {
        if(ptr->is_value()) {
            value = get_impl<T>(ptr);
            return  make_table();
        }
        return ptr->as_table();
    }


    class access {
        enum class Status {Default ,Require ,Parsed, Check };
    public:
        access(table_ptr table, const std::string& name="", Status status=Status::Parsed) : table_(table)
                                                                                          , name_(name)
                                                                                          , status_(status) {}
        template<typename T>
        access& element(T& target , const std::string& name) {
            if(status_ == Status::Check) { status_ = Status::Require; return *this; }
            if constexpr(is_table_v<T>) {
                auto tv = detail::panic(table_->get_table(name)
                                        , detail::toml_error_meesage(name, "table"));
                auto ac = access(tv, name);
                target.parse(ac);
            }
            else if constexpr(is_table_array_v<T>) {
                auto tv = detail::panic(table_->get_table_array(name)
                                       , detail::toml_error_meesage(name, "table_array"));
                std::for_each(tv->begin(), tv->end(), [&](auto t) {
                    detail::is_vector_e<T> table;
                    auto ac = access(t, name); target.parse(ac);
                    target.emplace_back(table);
                });
            }
            else if constexpr(is_nested_table_v<T>) {
                 this->for_each([&target](auto p) {
                        auto& table = target[p.first];
                        auto ac = access(p.second->as_table(), p.first);
                        table.parse(ac);
                 });
            }
            else if constexpr(is_list_table_v<T>) {
                 this->for_each([&target](auto p) {
                        auto name = p.first;
                        auto accs = access(p.second->as_table(), p.first);
                        auto result = std::find_if(target.begin(), target.end(), [&name](auto it) {
                                return it.name == name;
                        });
                        if(result != target->end()) {
                            result->parse(accs);
                        }else {
                            detail::is_list_e<T> lt;
                            lt.parse(accs);
                            target->push_back(lt);
                        }
                 });
            }
            else if constexpr(is_array_v<T>) {
                target = *detail::panic(table_->get_array_of<detail::is_vector_e<T>>(name)
                                       , detail::toml_error_meesage(name, "array"));
            }
            else if constexpr(is_inline_v<T>) {
                target.parse(*this);
            }
            else { // is_element
                if(auto tv = table_->get_as<T>(name)) {
                    target = tv.value();
                } else {
                    std::cerr << detail::toml_error_meesage(name, "element", name_) << std::endl;
                    exit(1);
                }
            }
            table_->erase(name);
            return *this;
        }

        template<typename T, typename U>
        inline access& element(std::optional<T>& target , const std::string& name, U default_value) {
            auto dfv = detail::only_str<T>(default_value);
            target = table_->get_as<T>(name).value_or(target ? target.value() : dfv);
            if(status_ != Status::Require) status_ = Status::Default;
            table_->erase(name);
            return *this;
        }

        template<typename T>
        access& element(std::optional<T>& target , const std::string& name) {
            if constexpr(is_table_v<T>) {
                if(auto tb = table_->get_table(name)){
                    target = target ? std::move(target) : T();
                    auto ac = access(tb, name);
                    target->parse(ac);
                }
                else {
                    auto def = table_->is_value() ? access(table_, name)
                                                  : access(make_table(), name, Status::Check);
                    target = target ? std::move(target) : T();
                    target->parse(def);
                    if(def.status_ != Status::Default) { target = std::nullopt; }
                }
            }
            else if constexpr(is_table_array_v<T>) {
                if(auto tv = table_->get_table_array(name)) {
                    target = target ? std::move(target) : T();
                    std::for_each(tv->begin(), tv->end(), [&target,&name](auto t) {
                        detail::is_vector_e<T> table;
                        auto ac = access(t, name);
                        table.parse(ac);
                        target->emplace_back(table);
                    });
                }
            }
            else if constexpr(is_nested_table_v<T>) {
                 if(auto tv = table_->get_table(name)) {
                    access(tv, name).for_each([&target](auto p) {
                        target = target ? std::move(target) : T();
                        auto& table = (*target)[p.first];
                        auto tb = p.second->is_value() ? make_table() : p.second->as_table();
                        auto accs = access(tb, p.first);
                        if(p.second->is_value()) accs.value_ = p.second;
                        table.parse(accs);
                    });
                 }
            }
            else if constexpr(is_list_table_v<T>) {
                 if(auto tv = table_->get_table(name)) {
                    access(tv, name).for_each([&target](auto p) {
                        target = target ? std::move(target) : T();
                        auto tb = p.second->is_value() ? make_table() : p.second->as_table();
                        auto accs = access(tb, p.first);
                        if(p.second->is_value()) accs.value_ = p.second;
                        auto& name =  p.first;
                        auto result = std::find_if(target->begin(), target->end(), [&name](auto it) {
                                return it.name == name;
                        });
                        if(result != target->end()) {
                            result->parse(accs);
                        }else {
                            detail::is_list_e<T> lt;
                            lt.parse(accs);
                            target->push_back(lt);
                        }
                    });
                 }
            }
            else if constexpr(is_array_v<T>) {
                target = table_->get_array_of<detail::is_vector_e<T>>(name);
            }
            else { // is_element
                target = target ? table_->get_as<T>(name).value_or(target.value())
                                : table_->get_as<T>(name);
            }
            table_->erase(name);
            return *this;
        }

        access& for_each(std::function<void(std::pair<std::string, base_ptr>)> func) {
            std::for_each(table_->begin(), table_->end(), [&func](auto tb) { func(tb); });
            return *this;
        }

        template<typename T>
        access& if_value(T& element, const std::string& name) {
            if(value_) {
                element = get_impl<detail::is_optional_e<T>>(value_).value();
            }
            else {
                this->element(element, name);
            }
            table_->erase(name);
            return *this;
        }

        template<typename T, typename U>
        access& if_value(std::optional<T>& element, const std::string& name, U default_value) {
            if(value_) {
                element = get_impl<T>(value_);
            }
            else {
                this->element(element, name, default_value);
            }
            table_->erase(name);
            return *this;
        }
        template<typename T>
        access& remains(std::optional<T>& target) {
            if constexpr (detail::is_vector_v<T>) {
                for_each([&target](auto it) {
                    auto [name_, evalue_] = it;
                    if(auto value_ = get_impl<detail::is_vector_e<T>>(evalue_)) {
                        target = target ? std::move(target) : T();
                        target->push_back(value_.value());
                    } else {
                        std::cerr << name_ <<" wrong type" << std::endl;
                    }
                });
            } else if constexpr (detail::is_map_v<T>) {
                for_each([&target](auto it) {
                    auto [name_, evalue_] = it;
                    if(auto value_ = get_impl<detail::is_map_e<T>>(evalue_)) {
                        target = target ? std::move(target) : T();
                        target->insert({name_, *value_});
                    } else {
                        std::cerr << name_ <<" wrong type" << std::endl;
                        exit(1);
                    }
                });
            } else {
                std::cerr << name_ <<" wrong type" << std::endl;
                exit(1);
            }
            return *this;
        }

        void no_remains() {
            for_each([this](auto it) {
                auto [name, table] = it;
                std::cerr << "unknown element \"" << name_ << "." << name  <<"\"" << std::endl;
                exit(1);
            });
        }

        const bool is_value() { return value_ != nullptr; }
        const std::string name() { return name_; }
        const table_ptr table()  { return table_; }
    private: 
        table_ptr table_;
        base_ptr value_;
        std::string name_;
        Status status_;
    };

    namespace detail {
        template<typename T> struct __dump__ {
            __dump__(T& d,const std::string& n, table_ptr table_) : def_(d), name_(n) {
                table = make_table();
                table->insert(name_, table_);
            }
            template<typename U> void parse(U defn) { defn.element(def_, name_); }
            T& def_;
            const std::string& name_;
            table_ptr table;
        };
    }

    template<typename T>
    table_ptr parser(std::optional<T>& def, table_ptr table, const std::string name="") {
        //std::cout << *table << std::endl;
        if constexpr (!is_table_v<T>) {
            def = def ? std::move(def) : T();
            detail::__dump__ dump_(def, name, table);
            auto ac = access(dump_.table, name);
            dump_.parse(ac);
        } else {
            def = def ? std::move(def) : T();
            auto ac = access(table, name);
            def->parse(ac);
        }
        return table;
    }

    template<typename T>
    inline table_ptr parser(std::optional<T>& def, const std::string& path, const std::string name="") {
        return parser(def, parse_file(path), name);
    }
}


#endif
