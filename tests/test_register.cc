#include "pico/class_factory.h"

class A
{
public:
    A() { std::cout << "A()" << std::endl; }
    ~A() { std::cout << "~A()" << std::endl; }

    virtual void foo() { std::cout << "A::foo()" << std::endl; }
};

class B : public A
{
public:
    B() { std::cout << "B()" << std::endl; }
    ~B() { std::cout << "~B()" << std::endl; }

    virtual void foo() override { std::cout << "B::foo()" << std::endl; }
};

class C : public A
{
public:
    C() { std::cout << "C()" << std::endl; }
    ~C() { std::cout << "~C()" << std::endl; }

    virtual void foo() override { std::cout << "C::foo()" << std::endl; }
};

REGISTER_CLASS(B);
REGISTER_CLASS(C);


int main(int argc, char const* argv[]) {
    auto a = pico::ClassFactory::Create("B");
    auto b = std::static_pointer_cast<A>(a);

    b->foo();

    a = pico::ClassFactory::Create("C");
    b = std::static_pointer_cast<A>(a);
    b->foo();

    return 0;
}
