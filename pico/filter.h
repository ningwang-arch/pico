#ifndef __PICO_FILTER_H__
#define __PICO_FILTER_H__


#include <memory>
#include <vector>

#include "class_factory.h"
#include "config.h"
#include "http/http.h"

namespace pico {
class FilterConfig;
class FilterChain;


class Filter
{
public:
    typedef std::shared_ptr<Filter> Ptr;
    Filter() {}

    virtual void init(const std::shared_ptr<FilterConfig>& config) {}

    virtual void doFilter(const pico::HttpRequest::Ptr& request, pico::HttpResponse::Ptr& response,
                          std::shared_ptr<FilterChain> chain) = 0;
};

class FilterConfig
{
public:
    typedef std::shared_ptr<FilterConfig> Ptr;
    FilterConfig() { m_filter = nullptr; }

    // setters
    void setName(const std::string& name) { m_name = name; }
    void setUrlPattern(const std::string& url_pattern) { m_url_pattern = url_pattern; }
    void setClass(const std::string& class_name) { m_class = class_name; }
    void setDescription(const std::string& description) { m_description = description; }
    void setFilter(const Filter::Ptr& filter) { m_filter = filter; }

    std::string getName() const { return m_name; }
    std::string getClass() const { return m_class; }
    std::string getUrlPattern() const { return m_url_pattern; }
    std::string getDescription() const { return m_description; }
    Filter::Ptr getFilter() const { return m_filter; }

    void addParam(const std::string& name, const std::string& value) {
        m_init_params[name] = value;
    }

    std::map<std::string, std::string> getInitParams() const { return m_init_params; }



    std::string toString() const {
        std::stringstream ss;
        ss << "FilterConfig: " << m_name << " " << m_class << " " << m_url_pattern << " "
           << m_description;
        return ss.str();
    }

private:
    std::string m_name;
    std::string m_url_pattern;
    std::string m_class;
    std::string m_description;
    std::map<std::string, std::string> m_init_params;

    Filter::Ptr m_filter;
};


class FilterChain : public std::enable_shared_from_this<FilterChain>
{
public:
    typedef std::shared_ptr<FilterChain> Ptr;


    FilterChain();
    FilterChain(const std::vector<FilterConfig::Ptr>& filters);
    FilterChain(const FilterChain& other);


    ~FilterChain();

    std::vector<FilterConfig::Ptr>& getFilters() { return m_filters; }

    void doFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response);

    void internalDoFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response);

    void reuse() { m_index = 0; }

    bool is_complete() { return m_index == m_size; }

    void addFilter(const FilterConfig::Ptr& filter_config);

    std::string toString() {
        std::stringstream ss;
        ss << "FilterChain: ";
        for (auto& filter : m_filters) { ss << filter->toString() << " "; }
        return ss.str();
    }

private:
    int m_index = 0;
    int m_size = 0;

    std::vector<FilterConfig::Ptr> m_filters;
};


template<>
class LexicalCast<std::string, FilterConfig::Ptr>
{
public:
    FilterConfig::Ptr operator()(const std::string& v) {
        YAML::Node node = YAML::Load(v);
        FilterConfig::Ptr config(new FilterConfig);
        config->setName(node["name"].as<std::string>());
        config->setClass(node["class"].as<std::string>());
        config->setUrlPattern(node["path"].as<std::string>());
        Filter::Ptr filter =
            std::static_pointer_cast<Filter>(ClassFactory::Instance().Create(config->getClass()));
        config->setFilter(filter);
        config->setDescription(node["description"].as<std::string>());
        if (node["init_params"].IsDefined()) {
            for (auto param : node["init_params"]) {
                config->addParam(param.first.as<std::string>(), param.second.as<std::string>());
            }
        }
        return config;
    }
};

template<>
class LexicalCast<FilterConfig::Ptr, std::string>
{
public:
    std::string operator()(const FilterConfig::Ptr& v) {
        YAML::Node node;
        node["name"] = v->getName();
        node["class"] = v->getClass();
        node["path"] = v->getUrlPattern();
        node["description"] = v->getDescription();
        for (auto& i : v->getInitParams()) { node["init_params"][i.first] = i.second; }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
}   // namespace pico

#endif