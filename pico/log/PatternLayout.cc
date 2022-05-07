#include "PatternLayout.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "PatternConverter.cc"


namespace pico {
PatternLayout::PatternLayout()
    : Layout() {
    init();
}

std::string PatternLayout::format(LogEvent::Ptr event) {
    std::stringstream ss;
    for (auto& converter : converters_) { ss << converter->convert(event); }
    return ss.str();
}

void PatternLayout::setPattern(const std::string& pattern) {
    pattern_ = pattern;
    init();
}

void PatternLayout::init() {
    converters_.clear();
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < pattern_.size(); i++) {
        if (pattern_[i] != '%') {
            nstr.append(1, pattern_[i]);
            continue;
        }
        if ((i + 1) < pattern_.size() && pattern_[i + 1] == '%') {
            nstr.append(1, '%');
            continue;
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while (n < pattern_.size()) {
            if (!fmt_status &&
                (!isalpha(pattern_[n]) && pattern_[n] != '{' && pattern_[n] != '}')) {
                str = pattern_.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0) {
                if (pattern_[n] == '{') {
                    str = pattern_.substr(i + 1, n - i - 1);
                    fmt_status = 1;   //解析格式
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            }
            else if (fmt_status == 1) {
                if (pattern_[n] == '}') {
                    fmt = pattern_.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == pattern_.size()) {
                if (str.empty()) { str = pattern_.substr(i + 1); }
            }
        }
        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.push_back(std::make_tuple(nstr, std::string(), 0));
                nstr.clear();
            }
            vec.push_back(std::make_tuple(str, fmt, 1));
            i = n - 1;
        }
        else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << pattern_ << " - " << pattern_.substr(i)
                      << std::endl;
            m_error = true;
            vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 0));
        }
    }
    if (!nstr.empty()) { vec.push_back(std::make_tuple(nstr, "", 0)); }

    static std::unordered_map<std::string,
                              std::function<PatternConverter::Ptr(const std::string& str)>>
        s_format_items = {
#define XX(str, C)                                                                     \
    {                                                                                  \
#        str, [](const std::string& fmt) { return PatternConverter::Ptr(new C(fmt)); } \
    }
            // %d %p [%c] - %m%n
            XX(m, MessagePatternConverter),
            XX(p, LevelPatternConverter),
            XX(c, NamePatternConverter),
            XX(t, ThreadIdPatternConverter),
            XX(n, NewLinePatternConverter),
            XX(d, DateTimePatternConverter),
            XX(f, FileNamePatternConverter),
            XX(l, LineNumberPatternConverter),
            XX(T, TabPatternConverter),
            XX(F, FiberIdPatternConverter),
            XX(N, ThreadNamePatternConverter),
#undef XX
        };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            converters_.push_back(
                PatternConverter::Ptr(new StringPatternConverter(std::get<0>(i))));
        }
        else {
            auto it = s_format_items.find(std::get<0>(i));
            if (it == s_format_items.end()) {
                std::cout << "pattern parse error: " << pattern_ << " - " << std::get<0>(i)
                          << std::endl;
                m_error = true;
                converters_.push_back(
                    PatternConverter::Ptr(new StringPatternConverter("<<pattern_error>>")));
            }
            else {
                converters_.push_back(it->second(std::get<1>(i)));
            }
        }
    }
}

}   // namespace pico
