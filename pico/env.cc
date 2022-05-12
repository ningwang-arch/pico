#include "env.h"

#include <getopt.h>
#include <string.h>
#include <unistd.h>

#include "logging.h"

extern int optind, opterr, optopt;
extern char* optargi;

static struct option long_options[] = {{"help", no_argument, NULL, 'h'},
                                       {"daemon", no_argument, NULL, 'd'},
                                       {"conf", required_argument, NULL, 'c'},
                                       {0, 0, 0, 0}};

static char short_options[] = "hdc:";

namespace pico {
bool Env::init(int argc, char* argv[]) {
    m_program = argv[0];
    m_exe = argv[0];
    m_cwd = getcwd(nullptr, 0);

    int c;
    set("argc", std::to_string(argc));
    while ((c = getopt_long(argc, argv, short_options, long_options, NULL)) != -1) {
        switch (c) {
        case 'h': set("h", "1"); break;
        case 'd': set("d", "1"); break;
        case 'c': set("c", optarg); break;
        default: return false;
        }
    }
    return true;
}

void Env::set(const std::string& key, const std::string& value) {
    Lock::WriteLock lock(_lock);
    _env[key] = value;
}

std::string Env::get(const std::string& key, const std::string& default_value) {
    Lock::ReadLock lock(_lock);
    auto it = _env.find(key);
    if (it != _env.end()) { return it->second; }
    return default_value;
}

bool Env::has(const std::string& key) {
    Lock::ReadLock lock(_lock);
    return _env.find(key) != _env.end();
}

void Env::remove(const std::string& key) {
    Lock::WriteLock lock(_lock);
    _env.erase(key);
}

void Env::addHelp(const std::string& key, const std::string& desc) {
    Lock::WriteLock lock(_lock);
    m_helps.push_back(std::make_pair(key, desc));
}

void Env::removeHelp(const std::string& key) {
    Lock::WriteLock lock(_lock);
    for (auto it = m_helps.begin(); it != m_helps.end(); ++it) {
        if (it->first == key) {
            m_helps.erase(it);
            return;
        }
    }
}

void Env::printHelp() {
    Lock::ReadLock lock(_lock);
    printf("Usage: %s [options]\n", m_program.c_str());
    for (auto it = m_helps.begin(); it != m_helps.end(); ++it) {
        auto pos = it->first.find(',');
        // -h, --help   help message
        if (pos != std::string::npos) {
            printf(" -%s, --%s\t%s\n",
                   it->first.substr(0, pos).c_str(),
                   it->first.substr(pos + 1).c_str(),
                   it->second.c_str());
        }
        else {
            printf(" -%s\t%s\n", it->first.c_str(), it->second.c_str());
        }
    }
}

bool Env::setEnv(const std::string& key, const std::string& val) {
    Lock::WriteLock lock(_lock);
    return setenv(key.c_str(), val.c_str(), 1) == 0;
}

std::string Env::getEnv(const std::string& key, const std::string& default_value) {
    Lock::ReadLock lock(_lock);
    char* val = getenv(key.c_str());
    if (val) { return val; }
    return default_value;
}

std::string Env::getAbsolutePath(const std::string& path) const {
    if (path.empty()) { return ""; }
    if (path[0] == '/') { return path; }

    return m_cwd + "/" + path;
}

std::string Env::getConfigPath() {
    return getAbsolutePath(get("c", "conf"));
}

}   // namespace pico