#include "config.h"
#include <fstream>

namespace pico {

ConfigVarBase::Ptr Config::LookupBase(const std::string& name) {
    RWMutexType::ReadLock lock(getMutex());
    auto it = getDatas().find(name);
    return it == getDatas().end() ? nullptr : it->second;
}

static void ListAllMember(const std::string& prefix, const YAML::Node& node,
                          std::list<std::pair<std::string, const YAML::Node>>& output) {
    // std::cout << node << std::endl;
    if (prefix.find_first_not_of("abcdefghijklmnopqrstuvwxyz._"
                                 "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") != std::string::npos) {
        LOG_ERROR("Config::Lookup() %s not found", prefix.c_str());
        return;
    }
    output.push_back(std::make_pair(prefix, node));

    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            ListAllMember(prefix.empty() ? it->first.Scalar() : prefix + "." + it->first.Scalar(),
                          it->second,
                          output);
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& root) {
    std::list<std::pair<std::string, const YAML::Node>> all_nodes;
    ListAllMember("", root, all_nodes);
    for (auto& i : all_nodes) {
        std::string key = i.first;
        if (key.empty()) { continue; }
        ConfigVarBase::Ptr var = LookupBase(key);

        if (var) {
            if (i.second.IsScalar()) { var->parse(i.second.Scalar()); }
            else {
                std::stringstream ss;
                ss << i.second;
                var->parse(ss.str());
            }
        }
    }
}

void Config::LoadFromFile(const std::string& filename) {
    std::ifstream fin(CONF_DIR + filename);
    if (!fin.is_open()) {
        LOG_ERROR("Config::LoadFromFile() %s not found", filename.c_str());
        return;
    }
    YAML::Node root = YAML::Load(fin);
    LoadFromYaml(root);
}

void Config::LoadFromConfDir(const std::string& conf_dir) {
    std::vector<std::string> files;
    std::string abs_path = getAbsolutePath(conf_dir);
    listDir(abs_path, files, ".yml");

    for (auto& i : files) { LoadFromFile(i); }
}

}   // namespace pico