#ifndef __PICO_LOG_TTCCLAYOUT_H__
#define __PICO_LOG_TTCCLAYOUT_H__

#include "Layout.h"

namespace pico {
class TTCCLayout : public Layout
{
public:
    TTCCLayout();
    virtual ~TTCCLayout() {}
    virtual std::string format(LogEvent::Ptr event);
};
}   // namespace pico

#endif