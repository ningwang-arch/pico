#include "pico/util.h"

#include <iostream>

class var_visitor : public boost::static_visitor<void>
{
public:
    template<typename T>
    void operator()(T& i) const {
        std::cout << typeid(T).name() << std::endl;
        std::cout << i << std::endl;
    }
};

void test() {
    std::string str = "/user/1/abc";
    std::string pattern = "/user/<int>/<string>";

    bool ret = pico::fuzzy_match(str, pattern);

    std::cout << "ret:" << ret << std::endl;

    auto variant = pico::split_variant(str, pattern);
    for (auto& v : variant) {
        boost::apply_visitor(var_visitor(), v);
    }
}

int main() {

    test();





    return 0;
}
