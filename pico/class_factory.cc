#include "class_factory.h"

#include <mutex>

#include "logging.h"

namespace pico {

ClassFactory& ClassFactory::Instance() {
    static ClassFactory factory;
    return factory;
}

void ClassFactory::Register(const std::string& name, Creator creator) {
    std::lock_guard<std::mutex> lock(getMutex());
    getClassMap()[name] = creator;
}

std::shared_ptr<void> ClassFactory::Create(const std::string& name) {
    std::lock_guard<std::mutex> lock(getMutex());
    auto it = getClassMap().find(name);
    if (it == getClassMap().end()) {
        LOG_ERROR("ClassFactory::Create() %s not found", name.c_str());
        return nullptr;
    }
    return it->second();
}

}   // namespace pico