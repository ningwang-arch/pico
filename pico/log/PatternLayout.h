#ifndef __PICO_LOG_PATTENLAYOUT_H__
#define __PICO_LOG_PATTENLAYOUT_H__

#include "Layout.h"
#include "PatternConverter.h"
#include <vector>

namespace pico {
class PatternLayout : public Layout
{
public:
    PatternLayout();
    virtual ~PatternLayout() {}
    virtual std::string format(LogEvent::Ptr event);

    void setPattern(const std::string& pattern);
    std::string getPattern() const { return pattern_; }

    void init();
    bool isError() const { return m_error; }

private:
    std::string pattern_ = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n";
    std::vector<PatternConverter::Ptr> converters_;
    bool m_error = false;
};
}   // namespace pico

#endif