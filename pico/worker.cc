#include "worker.h"

#include <boost/lexical_cast.hpp>
#include <cstdint>
#include <memory>
#include <string>

#include "config.h"

namespace pico {

pico::ConfigVar<std::map<std::string, std::map<std::string, std::string>>>::Ptr g_config_workers =
    pico::Config::Lookup("workers", std::map<std::string, std::map<std::string, std::string>>(),
                         "workers");

WorkerManager::WorkerManager()
    : m_stop(false) {}

void WorkerManager::add(IOManager::Ptr s) {
    m_datas[s->getName()].push_back(s);
}

IOManager::Ptr WorkerManager::get(const std::string& name) {
    auto it = m_datas.find(name);
    if (it == m_datas.end()) { return nullptr; }
    auto collect = it->second;

    return collect[rand() % collect.size()];
}


bool WorkerManager::init() {
    return init(g_config_workers->getValue());
}

bool WorkerManager::init(std::map<std::string, std::map<std::string, std::string>> config) {
    for (auto& it : config) {
        auto name = it.first;
        auto conf = it.second;

        uint32_t thread_num = conf.find("thread_num") != conf.end()
                                  ? boost::lexical_cast<uint32_t>(conf.find("thread_num")->second)
                                  : 1;
        uint32_t worker_num = conf.find("worker_num") != conf.end()
                                  ? boost::lexical_cast<uint32_t>(conf.find("worker_num")->second)
                                  : 1;

        for (uint32_t i = 0; i < worker_num; i++) {
            IOManager::Ptr s(
                new IOManager(thread_num, false, name + (i == 0 ? "" : "-" + std::to_string(i))));
            add(s);
        }
    }
    m_stop = m_datas.empty();

    return true;
}

void WorkerManager::stop() {
    if (m_stop) { return; }

    for (auto& i : m_datas) {
        for (auto& item : i.second) {
            item->schedule([]() {});
            item->stop();
        }
    }
    m_datas.clear();
    m_stop = true;
}

int WorkerManager::getWorkerNum() {
    return m_datas.size();
}

}   // namespace pico
