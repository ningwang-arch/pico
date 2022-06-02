#ifndef __PICO_MAPPER_MAPPER_EXCEPTION_H__
#define __PICO_MAPPER_MAPPER_EXCEPTION_H__

#include <exception>
#include <string>

namespace pico {
class MapperException : std::exception
{
public:
    MapperException(const std::string msg)
        : msg_(msg) {}

    virtual const char* what() const throw() { return msg_.c_str(); }
    virtual ~MapperException() throw() {}

private:
    const std::string msg_;
};

}   // namespace pico


#endif   // !__PICO_MAPPER_MAPPER_EXCEPTION_H__
