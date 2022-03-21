#ifndef __PICO_LOG_SIMPLELAYOUT_H__
#define __PICO_LOG_SIMPLELAYOUT_H__

#include "Layout.h"
#include "LogEvent.h"

namespace pico {
class SimpleLayout : public Layout
{
public:
    SimpleLayout();
    virtual ~SimpleLayout() {}
    virtual std::string format(LogEvent::Ptr event);
};
}   // namespace pico

#endif