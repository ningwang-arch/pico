#include "SimpleLayout.h"

#include <sstream>

namespace pico {

SimpleLayout::SimpleLayout()
    : Layout() {}

std::string SimpleLayout::format(LogEvent::Ptr event) {
    std::stringstream ss;
    ss << LogLevel::toString(event->getLevel()) << " - " << event->getMessage() << std::endl;
    ;
    return ss.str();
}
}   // namespace pico