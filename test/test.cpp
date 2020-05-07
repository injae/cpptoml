#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "tomlpp/orm.hpp"
#include <cstdlib>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

#define TOML_D(...) __VA_ARGS__, #__VA_ARGS__

struct dep : public toml::orm::table {
    template<typename Define>
    void parse(Define& defn) {
        name = defn.name();
        defn.if_value(TOML_D(version))
            .element(TOML_D(level), 0)
            .element(TOML_D(on), false)
            .remains(remains);
    }

    std::string name;
    opt<int>    level;
    string version;
    opt<bool>   on;
    opt<nested<string>> remains;
};

struct deps : public toml::orm::table {
    template<typename Define>
    void parse(Define& d) {
        d.for_each([this](auto p) {
            dep table;
            list = list ? std::move(list) : arr<dep>();  // require init 
            auto ptr = toml::orm::table_or_value(p.second, table.level);
            auto ac = toml::orm::access(ptr, p.first);
            table.parse(ac);
            list->emplace_back(std::move(table));
        });
    }
    opt<arr<dep>> list;
};

struct tg : public toml::orm::table {
    template<typename Define>
    void parse(Define& defn) {
        defn.element(TOML_D(name))
            .element(TOML_D(level), 10);
        type = defn.name();
    }
    string name;
    string type;
    opt<int> level;
};

struct config: public toml::orm::table {
    template<typename Define>
    void parse(Define& defn) {
        opt<tg> lib;
        defn.element(TOML_D(name))
            .element(TOML_D(nums))
            .element(tgs, "bin")
            .element(TOML_D(lib))
            .element(TOML_D(tt), "ss")
            .element(TOML_D(deps))
            .no_remains();
        if(lib) {tgs->push_back(lib.value());}
    }
    string name;
    opt<std::list<dep>> deps;
    opt<arr<int64_t>> nums;
    opt<arr<tg>> tgs;
    opt<string> tt;
};

TEST_CASE("Get env test", "[test env]") {
    auto conf = std::make_optional<config>();
    auto path = fs::current_path();
    REQUIRE((path/"../test/test.toml").lexically_normal() == "/Users/nieel/dev/cppm/libs/tomlpp/test/test.toml");
}
