#include "pico/config.h"
#include <iostream>


pico::ConfigVar<int>::Ptr g_int_var = pico::Config::Lookup<int>(std::string("int"), 0, "int var");


class Servlet
{
public:
    Servlet() {}

    std::string name;
    std::string pattern;
    std::string cls;

    std::string toString() { return name + " " + pattern + " " + cls; }
};

namespace pico {
template<>
class LexicalCast<std::string, Servlet>
{
public:
    Servlet operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        Servlet servlet;
        servlet.name = node["name"].as<std::string>();
        servlet.pattern = node["pattern"].as<std::string>();
        servlet.cls = node["class"].as<std::string>();
        return servlet;
    }
};

template<>
class LexicalCast<Servlet, std::string>
{
public:
    std::string operator()(const Servlet& v) {
        YAML::Node node;
        node["name"] = v.name;
        node["pattern"] = v.pattern;
        node["class"] = v.cls;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}   // namespace pico

void test_config() {
    std::cout << "g_int_var: " << g_int_var->getValue() << std::endl;
    pico::Config::LoadFromFile("web.yml");
    std::cout << "g_int_var: " << g_int_var->getValue() << std::endl;
}


pico::ConfigVar<std::vector<Servlet>>::Ptr g_servlets = pico::Config::Lookup<std::vector<Servlet>>(
    std::string("servlet"), std::vector<Servlet>(), "servlets");

void test_self() {
    std::cout << "g_servlets: " << g_servlets->getValue().size() << std::endl;
    pico::Config::LoadFromFile("web.yml");
    std::cout << "g_servlets: " << g_servlets->getValue().size() << std::endl;

    for (auto s : g_servlets->getValue()) { std::cout << s.toString() << std::endl; }
}

int main(int argc, char const* argv[]) {
    // test_config();
    test_self();
    return 0;
}
