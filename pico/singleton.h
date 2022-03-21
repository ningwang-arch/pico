#ifndef __PICO_SINGLETON_H__
#define __PICO_SINGLETON_H__

namespace pico {

template<class T, class X = void, int N = 0>
class Singleton
{
private:
    /* data */

public:
    static T* getInstance() {
        static T v;
        return &v;
    }
};

template<class T, class X = void, int N = 0>
class SingletonPtr
{
public:
    static std::shared_ptr<T> getInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};
};   // namespace pico

#endif