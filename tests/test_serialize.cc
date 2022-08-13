#include "../pico/serialize.hpp"
#include "../pico/util.h"

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
        os << "}}";

        return os;
    }
};

int main(int argc, char const* argv[]) {
    A a;
    a.a = 1;
    a.b = 2;
    std::string ret = a.encode();

    std::cout << ret << std::endl;

    A a2;
    bool r = a2.decode(ret);

    if (r) {
        std::cout << a2 << std::endl;
    }

    B b;
    b.str = "hello";
    b.a = a;
    b.vec.push_back(a);
    b.map["a"] = a;
    b.map["b"] = a;
    ret = b.encode();

    std::cout << ret << std::endl;


    B b1;
    r = b1.decode(ret);
    if (r) {
        std::cout << b1 << std::endl;
    }


    return 0;
}
