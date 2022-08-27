#include "../pico/util.h"

#include "../pico/serialize.hpp"
struct A
{
    int a;
    int b;

    REFLECTION(A, a, b);

    bool operator==(const A& other) const { return a == other.a && b == other.b; }

    friend std::ostream& operator<<(std::ostream& os, const A& a) {
        os << "A(" << a.a << ", " << a.b << ")";
        return os;
    }
};

struct B
{
    std::string str;
    A a;

    std::vector<A> vec;

    std::map<std::string, A> map;


    REFLECTION(B, str, a, vec, map);



    friend std::ostream& operator<<(std::ostream& os, const B& b) {
        os << "B(" << b.str << ", " << b.a << ", [";
        for (auto& i : b.vec) {
            os << i << (i == b.vec.back() ? "" : ", ");
        }
        os << "], {";
        for (auto& i : b.map) {
            os << i.first << ": " << i.second << ", ";
        }
        os << "})";

        return os;
    }
};


void test_01() {
    A a;
    a.a = 101;
    a.b = 23;

    std::cout << a.encode() << std::endl;

    auto ret = pico::serialize(a);
    std::cout << ret << std::endl;

    A a2 = pico::deserialize<A>(ret);

    std::cout << a2 << std::endl;

    std::map<std::string, int> map;
    map["a"] = 1;
    map["b"] = 2;
    ret = pico::serialize(map);
    std::cout << ret << std::endl;
    std::map<std::string, int> map2;

    auto map1 = pico::deserialize<std::map<std::string, int>>(ret);
    for (auto& i : map1) {
        std::cout << i.first << ": " << i.second << std::endl;
    }

    std::vector<B> vec;
    B b;
    b.str = "hello";
    b.a = a;
    b.vec.push_back(a);
    b.map["a"] = a;

    auto ret2 = pico::serialize(b);
    std::cout << ret2 << std::endl;

    B b2 = pico::deserialize<B>(ret2);
    std::cout << b2 << std::endl;
}

int main(int argc, char const* argv[]) {
    test_01();
    return 0;
}
