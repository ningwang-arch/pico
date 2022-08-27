#ifndef __PICO_JWT_EXPECT_H__
#define __PICO_JWT_EXPECT_H__

#include <exception>
#include <string>
#include <string_view>

namespace pico {

class VerificationException : public std::exception
{
public:
    explicit VerificationException(const std::string& message)
        : m_message(message) {}
    virtual ~VerificationException() {}
    virtual const char* what() const noexcept { return m_message.c_str(); }

private:
    std::string m_message;
};
}   // namespace pico
#endif