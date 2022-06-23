#ifndef __PICO_MAPPER_SQL_SQL_OPTION_H__
#define __PICO_MAPPER_SQL_SQL_OPTION_H__

#include <string>
#include <unordered_map>

#include "../../config.h"

namespace pico {
struct SQLOption
{
    std::string type = "";
    std::string host = "";
    int port = 0;
    std::string user = "";
    std::string password = "";
    std::string database = "";
    std::string charset = "";
    int connect_timeout = 0;
    int reconnect_time = 0;
};

template<>
class LexicalCast<std::string, std::unordered_map<std::string, SQLOption>>
{
public:
    std::unordered_map<std::string, SQLOption> operator()(const std::string& str) {
        std::unordered_map<std::string, SQLOption> ret;
        YAML::Node node = YAML::Load(str);
        for (auto i : node) {
            SQLOption option;
            option.type = i["type"].as<std::string>(option.type);
            option.host = i["host"].as<std::string>(option.host);
            option.port = i["port"].as<int>(option.port);
            option.user = i["user"].as<std::string>(option.user);
            option.password = i["password"].as<std::string>(option.password);
            option.database = i["database"].as<std::string>(option.database);
            option.charset = i["charset"].as<std::string>(option.charset);
            option.connect_timeout = i["connect_timeout"].as<int>(option.connect_timeout);
            option.reconnect_time = i["reconnect_time"].as<int>(option.reconnect_time);
            ret.insert(std::make_pair(i["name"].as<std::string>(), option));
        }
        return ret;
    }
};

template<>
class LexicalCast<std::unordered_map<std::string, SQLOption>, std::string>
{
public:
    std::string operator()(const std::unordered_map<std::string, SQLOption>& v) {
        YAML::Node node(YAML::NodeType::Sequence);
        for (auto& i : v) {
            YAML::Node option;
            option["name"] = i.first;
            option["type"] = i.second.type;
            option["host"] = i.second.host;
            option["port"] = i.second.port;
            option["user"] = i.second.user;
            option["password"] = i.second.password;
            option["database"] = i.second.database;
            option["charset"] = i.second.charset;
            option["connect_timeout"] = i.second.connect_timeout;
            option["reconnect_time"] = i.second.reconnect_time;
            node.push_back(option);
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};



}   // namespace pico

#endif