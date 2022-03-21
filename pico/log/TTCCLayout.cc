#include "TTCCLayout.h"

#include <sstream>

#include "pico/util.h"

namespace pico {

TTCCLayout::TTCCLayout()
    : Layout() {}

std::string TTCCLayout::format(LogEvent::Ptr event) {
    std::stringstream ss;
    std::string format = "%Y-%m-%d %H:%M:%S";
    ss << getForamtedTime(format.c_str()) << " [" << event->getThreadName() << "] "
       << LogLevel::toString(event->getLevel()) << " - " << event->getMessage() << std::endl;
    return ss.str();
}

}   // namespace pico