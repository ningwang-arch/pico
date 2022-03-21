#ifndef __PICO_LOG_PATTERNCONVERTER_H__
#define __PICO_LOG_PATTERNCONVERTER_H__

#include <memory>

#include "LogEvent.h"

namespace pico {

class PatternConverter
{
public:
    typedef std::shared_ptr<PatternConverter> Ptr;
    PatternConverter(const std::string& pattern = ""){};
    virtual ~PatternConverter() {}
    virtual std::string convert(LogEvent::Ptr event) = 0;
};

}   // namespace pico

#endif