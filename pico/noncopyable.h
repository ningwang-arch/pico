#ifndef __PICO_NOCOPYABLE_H__
#define __PICO_NOCOPYABLE_H__

namespace pico {
class Noncopyable
{
public:
    Noncopyable() = default;
    ~Noncopyable() = default;

    Noncopyable(const Noncopyable&) = delete;

    Noncopyable& operator=(const Noncopyable&) = delete;
};
}   // namespace pico

#endif