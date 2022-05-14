#ifndef __PICO_WORKER_H
#define __PICO_WORKER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "iomanager.h"
#include "singleton.h"

namespace pico {
class WorkerManager
{
public:
    typedef std::shared_ptr<WorkerManager> Ptr;
    WorkerManager();

    void add(IOManager::Ptr s);

    IOManager::Ptr get(const std::string& name);

    bool init();

    bool init(std::map<std::string, std::map<std::string, std::string>>);

    void stop();

    int getWorkerNum();

private:
    std::map<std::string, std::vector<IOManager::Ptr>> m_datas;
    bool m_stop;
};

typedef Singleton<WorkerManager> WorkerMgr;
}   // namespace pico

#endif