#include "pico/date.h"

#include <iostream>

void test() {
    pico::Date date;
    date.setMonth(2);
    date.setDay(31);
    std::cout << date.to_string() << std::endl;
}

int main(int argc, char const* argv[]) {
    test();
    return 0;
}
