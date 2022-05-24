#include "pico/mustache.h"

#include <json/json.h>

#include "pico/util.h"

#define ASSERT_EQ(name, a, b)                                                          \
    if ((a) != (b)) {                                                                  \
        std::cerr << name << " Assertion failed: " << #a << " != " << #b << std::endl; \
        std::cerr << "  " << a << " != " << b << std::endl;                            \
        exit(1);                                                                       \
    }

static Json::Value partial = Json::Value(Json::objectValue);

std::string loader(const std::string& name) {
    // std::cout << partial[name].toStyledString() << std::endl;
    if (partial[name].type() == Json::nullValue) { return ""; }
    std::string ret = partial[name].toStyledString();
    ret = ret.substr(1, ret.size() - 3);
    // replace "\n" to \n, "\r" to \r, "\t" to \t
    std::string::size_type pos = 0;
    while ((pos = ret.find("\\n", pos)) != std::string::npos) {
        ret.replace(pos, 2, "\n");
        pos += 1;
    }
    pos = 0;
    while ((pos = ret.find("\\r", pos)) != std::string::npos) {
        ret.replace(pos, 2, "\r");
        pos += 1;
    }
    pos = 0;
    while ((pos = ret.find("\\t", pos)) != std::string::npos) {
        ret.replace(pos, 2, "\t");
        pos += 1;
    }
    return ret;
    // return partial[name].toStyledString();
}

Json::Value load_tests(const std::string& filename) {
    Json::Value tests;
    std::ifstream file(filename);
    file >> tests;
    return tests["tests"];
}

std::string read_all(const std::string& filename) {
    std::ifstream is(filename);
    return {std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>()};
}

void test() {
    Json::Value root;
    root["message"] = "Hello, World!";
    root["boolean"] = false;
    std::string str = pico::mustache::load("index.tpl").render_string(root);
    std::cout << str << std::endl;
}

void test_without_partials(const std::string& filename) {
    Json::Value root = load_tests("templates/" + filename);
    for (auto& test : root) {
        std::string name = test["name"].asString();
        std::string desc = test["desc"].asString();
        Json::Value ctx = test["data"];
        std::string tpl = test["template"].asString();

        if (test.isMember("partials")) { partial = test["partials"]; }


        std::string expected = test["expected"].asString();
        std::string ret = pico::mustache::compile(tpl).render_string(ctx);
        ASSERT_EQ(name, ret, expected);
    }
    std::cout << "test " << filename << " passed" << std::endl;
}

void test_partials() {}

int main(int argc, char const* argv[]) {
    // test();
    pico::mustache::set_loader(loader);
    std::string without_partials[] = {"comments.json",
                                      "delimiters.json",
                                      "interpolation.json",
                                      "inverted.json",
                                      "sections.json",
                                      "partials.json"};
    for (auto& filename : without_partials) { test_without_partials(filename); }
    // test_comments();
    return 0;
}
