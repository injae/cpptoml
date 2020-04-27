#include "tomlpp/orm.hpp"

using namespace std;

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
    opt<nested<dep>> deps;
    opt<arr<int64_t>> nums;
    opt<arr<tg>> tgs;
    opt<string> tt;
};

int main() {
    auto conf = std::make_optional<config>();
    toml::orm::parser(conf, "test.toml", "config");
    //cout << conf->deps->list->empty() << endl;

    for(auto& [_,x]: conf->deps.value()) {
        cout << x.name << x.version << *x.level << endl;
        for(auto& [y, z]: x.remains.value()) {
            cout << y << ":" << z << " ";
        }
        cout << endl;
    }

    return 0;
}
