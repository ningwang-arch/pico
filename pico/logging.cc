#include "logging.h"

#include <unordered_set>

#include "config.h"
#include "log/LogAppender.h"

struct AppenderDefine
{
    int type;   // 0: file, 1: console
    std::string filename;
    int layout;   // 0: pattern, 1: simple, 2: ttcc
    std::string pattern;

    bool operator==(const AppenderDefine& other) const {
        return type == other.type && filename == other.filename && layout == other.layout &&
               pattern == other.pattern;
    }

    bool operator!=(const AppenderDefine& other) const { return !(*this == other); }

    bool operator<(const AppenderDefine& other) const {
        if (type != other.type) { return type < other.type; }
        if (filename != other.filename) { return filename < other.filename; }
        if (layout != other.layout) { return layout < other.layout; }
        return pattern < other.pattern;
    }

    bool operator>(const AppenderDefine& other) const { return other < *this; }
};

struct LogDefine
{
    std::string name;
    pico::LogLevel::Level level = pico::LogLevel::UNKNOWN;
    std::vector<AppenderDefine> appenders;

    bool operator==(const LogDefine& other) const {
        return name == other.name && level == other.level && appenders == other.appenders;
    }

    bool operator!=(const LogDefine& other) const { return !(*this == other); }

    bool operator<(const LogDefine& other) const { return name < other.name; }

    bool operator>(const LogDefine& other) const { return name > other.name; }
};

namespace pico {
template<>
class LexicalCast<std::string, LogDefine>
{
public:
    LogDefine operator()(const std::string& str) const {
        LogDefine log;
        YAML::Node node = YAML::Load(str);
        if (!node["name"].IsDefined()) { throw std::runtime_error("LogDefine::name not defined"); }
        log.name = node["name"].as<std::string>();
        log.level = pico::LogLevel::fromString(
            node["level"].IsDefined() ? node["level"].as<std::string>().c_str() : "");
        if (node["appenders"].IsDefined()) {
            for (auto appender : node["appenders"]) {
                AppenderDefine appender_define;
                auto type = appender["type"].as<std::string>();
                if (type == "console") { appender_define.type = 1; }
                else {
                    appender_define.type = 0;
                    if (!appender["filename"].IsDefined()) {
                        throw std::runtime_error("LogDefine::filename not defined");
                    }
                    appender_define.filename = appender["filename"].as<std::string>();
                }
                auto layout = appender["layout"].as<std::string>();
                if (layout == "pattern") {
                    appender_define.layout = 0;
                    if (appender["pattern"].IsDefined()) {
                        appender_define.pattern = appender["pattern"].as<std::string>();
                    }
                }
                else if (layout == "simple") {
                    appender_define.layout = 1;
                }
                else if (layout == "ttcc") {
                    appender_define.layout = 2;
                }
                else {
                    appender_define.layout = 1;
                }
                log.appenders.push_back(appender_define);
            }
        }
        return log;
    }
};

template<>
class LexicalCast<LogDefine, std::string>
{
public:
    std::string operator()(const LogDefine& log) const {
        std::stringstream ss;
        YAML::Node node;
        node["name"] = log.name;
        node["level"] = pico::LogLevel::toString(log.level);
        for (auto appender : log.appenders) {
            YAML::Node appender_node;
            if (appender.type == 0) {
                appender_node["type"] = "file";
                appender_node["filename"] = appender.filename;
            }
            else {
                appender_node["type"] = "console";
            }
            if (appender.layout == 0) {
                appender_node["layout"] = "pattern";
                appender_node["pattern"] = appender.pattern;
            }
            else if (appender.layout == 1) {
                appender_node["layout"] = "simple";
            }
            else if (appender.layout == 2) {
                appender_node["layout"] = "ttcc";
            }
            node["appenders"].push_back(appender_node);
        }
        ss << node;
        return ss.str();
    }
};
}   // namespace pico

static pico::ConfigVar<std::set<LogDefine>>::Ptr g_log_defines =
    pico::Config::Lookup<std::set<LogDefine>>("logs", std::set<LogDefine>(), "logs config");

LogInit::LogInit() {
    std::set<LogDefine> log_defines = g_log_defines->getValue();
    for (auto log_define : log_defines) {
        pico::Logger::Ptr logger = std::make_shared<pico::Logger>(log_define.name);
        logger->setLevel(log_define.level);
        for (auto appender : log_define.appenders) {
            pico::LogAppender::Ptr log_appender;
            if (appender.type == 0) {
                log_appender = std::make_shared<pico::FileAppender>(appender.filename);
            }
            else {
                log_appender = std::make_shared<pico::StdoutAppender>();
            }
            if (appender.layout == 0) {
                pico::PatternLayout::Ptr layout = std::make_shared<pico::PatternLayout>();
                if (!appender.pattern.empty()) { layout->setPattern(appender.pattern); }
                log_appender->setLayout(layout);
            }
            else if (appender.layout == 1) {
                log_appender->setLayout(std::make_shared<pico::SimpleLayout>());
            }
            else if (appender.layout == 2) {
                { log_appender->setLayout(std::make_shared<pico::TTCCLayout>()); }
            }
            logger->addAppender(log_appender);
        }
        LoggerMgr::getInstance()->addLogger(log_define.name, logger);
    }
}