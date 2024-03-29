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
    virtual std::string format(LogEvent::Ptr event) override;

    void setPattern(const std::string& pattern) override;
    std::string getPattern() const override { return pattern_; }

    void init();
    bool isError() const { return m_error; }

private:
    std::string pattern_ = "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%T%m%n";
    std::vector<PatternConverter::Ptr> converters_;
    bool m_error = false;
};
}   // namespace pico

#endif
