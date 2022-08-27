#ifndef __CLASS_FACTORY_H__
#define __CLASS_FACTORY_H__

#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace pico {

typedef std::function<std::shared_ptr<void>(void)> Creator;

typedef std::unordered_map<std::string, Creator> ClassMap;

class ClassFactory
{
public:
    static ClassFactory& Instance();
    static void Register(const std::string& name, Creator creator);
    static std::shared_ptr<void> Create(const std::string& name);

private:
    ClassFactory() {}

    static ClassMap& getClassMap() {
        static ClassMap classMap;
        return classMap;
    }

    static std::mutex& getMutex() {
        static std::mutex mutex;
        return mutex;
    }
};

class RegisterClass
{
public:
    RegisterClass(const std::string& name, Creator creator) {
        ClassFactory::Instance().Register(name, creator);
    }
};


}   // namespace pico

#define REGISTER_CLASS(class_name)                          \
    static pico::RegisterClass register_class_##class_name( \
        #class_name, []() { return std::make_shared<class_name>(); });

#endif