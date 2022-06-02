#ifndef __PICO_MAPPER_SQL_SQLEXCEPTION_H__
#define __PICO_MAPPER_SQL_SQLEXCEPTION_H__

#include <exception>
#include <stdexcept>
#include <string>

namespace pico {

class SQLException : public std::runtime_error
{
public:
    explicit SQLException(const std::string& msg)
        : std::runtime_error(msg) {}
};

}   // namespace pico

#endif