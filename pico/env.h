#ifndef __PICO_ENV_H__
#define __PICO_ENV_H__

#include <memory>
#include <unordered_map>
#include <vector>

#include "mutex.h"
#include "singleton.h"

namespace pico {

class Env
{
public:
    typedef std::shared_ptr<Env> Ptr;
    typedef RWMutex Lock;

    bool init(int argc, char* argv[]);

    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key, const std::string& default_value = "");
    bool has(const std::string& key);
    void remove(const std::string& key);

    void addHelp(const std::string& key, const std::string& desc);
    void removeHelp(const std::string& key);
    void printHelp();

    const std::string& getExe() const { return m_exe; }
    const std::string& getCwd() const { return m_cwd; }

    bool setEnv(const std::string& key, const std::string& val);
    std::string getEnv(const std::string& key, const std::string& default_value = "");

    std::string getAbsolutePath(const std::string& path) const;
    std::string getConfigPath();


private:
    Lock _lock;

    std::unordered_map<std::string, std::string> _env;
    std::vector<std::pair<std::string, std::string>> m_helps;

    std::string m_program;
    std::string m_exe;
    std::string m_cwd;
};

typedef Singleton<Env> EnvManager;

}   // namespace pico

#endif