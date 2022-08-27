#include "PatternConverter.h"

#include "Logger.h"
#include "pico/util.h"

namespace pico {
class MessagePatternConverter : public PatternConverter
{
public:
    MessagePatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override { return event->getMessage(); }
};

class LevelPatternConverter : public PatternConverter
{
public:
    LevelPatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override {
        return LogLevel::toString(event->getLevel());
    }
};

class NamePatternConverter : public PatternConverter
{
public:
    NamePatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override {
        return event->getLogger()->getName();
    }
};

class ThreadIdPatternConverter : public PatternConverter
{
public:
    ThreadIdPatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override {
        return std::to_string(event->getThreadId());
    }
};

class ThreadNamePatternConverter : public PatternConverter
{
public:
    ThreadNamePatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override { return event->getThreadName(); }
};

class FiberIdPatternConverter : public PatternConverter
{
public:
    FiberIdPatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override {
        return std::to_string(event->getFiberId());
    }
};

class DateTimePatternConverter : public PatternConverter
{
public:
    DateTimePatternConverter(const std::string& pattern = "%Y-%m-%d %H:%M:%S")
        : m_pattern(pattern) {
        if (m_pattern.empty()) {
            m_pattern = "%Y-%m-%d %H:%M:%S";
        }
    }
    virtual std::string convert(LogEvent::Ptr event) override {
        return getForamtedTime(m_pattern.c_str());
    }

private:
    std::string m_pattern;
};

class FileNamePatternConverter : public PatternConverter
{
public:
    FileNamePatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override { return event->getFile(); }
};

class LineNumberPatternConverter : public PatternConverter
{
public:
    LineNumberPatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override {
        return std::to_string(event->getLine());
    }
};
class NewLinePatternConverter : public PatternConverter
{
public:
    NewLinePatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override { return "\n"; }
};

class StringPatternConverter : public PatternConverter
{
public:
    StringPatternConverter(const std::string& pattern = "")
        : m_str(pattern) {}
    virtual std::string convert(LogEvent::Ptr event) override { return m_str; }

private:
    std::string m_str;
};

class TabPatternConverter : public PatternConverter
{
public:
    TabPatternConverter(const std::string& pattern = "") {}
    virtual std::string convert(LogEvent::Ptr event) override { return "\t"; }
};

}   // namespace pico