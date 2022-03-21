#include "LogLevel.h"

#include <string.h>
#include <string>

namespace pico {

const char* LogLevel::toString(LogLevel::Level level) {
    switch (level) {
    case UNKNOWN: return "UNKNOWN";
    case DEBUG: return "DEBUG";
    case INFO: return "INFO";
    case WARN: return "WARN";
    case ERROR: return "ERROR";
    case FATAL: return "FATAL";
    default: return "UNKNOWN";
    }
}

LogLevel::Level LogLevel::fromString(const char* level) {
    if (strcasecmp(level, "UNKNOWN") == 0) { return UNKNOWN; }
    else if (strcmp(level, "DEBUG") == 0) {
        return DEBUG;
    }
    else if (strcasecmp(level, "INFO") == 0) {
        return INFO;
    }
    else if (strcasecmp(level, "WARN") == 0) {
        return WARN;
    }
    else if (strcasecmp(level, "ERROR") == 0) {
        return ERROR;
    }
    else if (strcasecmp(level, "FATAL") == 0) {
        return FATAL;
    }
    else {
        return UNKNOWN;
    }
}
}   // namespace pico