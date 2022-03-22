#ifndef __PICO_LOG_LAYOUT_H__
#define __PICO_LOG_LAYOUT_H__

#include "LogEvent.h"
#include <memory>
#include <string>
namespace pico {
class Layout
{
public:
    typedef std::shared_ptr<Layout> Ptr;
    Layout(){};
    virtual ~Layout() {}
    virtual void setPattern(const std::string& pattern) {}
    virtual std::string getPattern() const { return nullptr; }
    virtual std::string format(LogEvent::Ptr event) = 0;
};
}   // namespace pico

#endif