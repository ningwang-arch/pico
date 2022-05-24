#ifndef __PICO_FILTER_H__
#define __PICO_FILTER_H__


#include <memory>
#include <vector>

#include "class_factory.h"
#include "config.h"
#include "http/http.h"
#include "http/servlet.h"

namespace pico {
class FilterConfig;
class FilterChain;

struct InsertByLength
{
    bool operator()(const std::string& a, const std::string& b) const {
        return a.size() >= b.size();
    }
};

class Filter
{
public:
    typedef std::shared_ptr<Filter> Ptr;
    Filter() {}

    virtual void init(const std::shared_ptr<FilterConfig>& config) {}

    virtual void doFilter(const pico::HttpRequest::Ptr& request, pico::HttpResponse::Ptr& response,
                          std::shared_ptr<pico::FilterChain> chain) = 0;

    virtual void destroy() {}
};

class FilterConfig
{
public:
    typedef std::shared_ptr<FilterConfig> Ptr;
    FilterConfig() { m_filter = nullptr; }

    ~FilterConfig() {
        if (m_filter) {
            m_filter->destroy();
            m_filter = nullptr;
        }
    }

    // setters
    void setName(const std::string& name) { m_name = name; }
    void setClass(const std::string& class_name) { m_class = class_name; }
    void setDescription(const std::string& description) { m_description = description; }
    void setFilter(const Filter::Ptr& filter) { m_filter = filter; }

    std::string getName() const { return m_name; }
    std::string getClass() const { return m_class; }
    std::string getDescription() const { return m_description; }
    Filter::Ptr getFilter() const { return m_filter; }

    void addParam(const std::string& name, const std::string& value) {
        m_init_params[name] = value;
    }

    void setInitParams(const std::map<std::string, std::string>& init_params) {
        m_init_params = init_params;
    }

    std::map<std::string, std::string> getInitParams() const { return m_init_params; }

    std::string toString() const {
        std::stringstream ss;
        ss << "FilterConfig: " << m_name << " " << m_class << " " << m_description;
        return ss.str();
    }

private:
    std::string m_name;
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
    FilterChain(const FilterChain& other);


    ~FilterChain();

    std::vector<FilterConfig::Ptr>& getFilters() { return m_filters; }
    void setFilters(const std::vector<FilterConfig::Ptr>& filters) {
        m_filters = filters;
        m_size = m_filters.size();
    }


    void doFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response);

    void internalDoFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response);

    void reuse() { m_index = 0; }

    void addFilter(const FilterConfig::Ptr& filter_config);

    void setServlet(const Servlet::Ptr& servlet) { m_servlet = servlet; }

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
    Servlet::Ptr m_servlet;
};

struct FilterConfs
{
    std::string name;
    std::string cls_name;
    std::string description;
    std::vector<std::string> url_patterns;
    std::map<std::string, std::string> init_params;

    bool operator==(const FilterConfs& other) const {
        return name == other.name && cls_name == other.cls_name &&
               description == other.description && url_patterns == other.url_patterns &&
               init_params == other.init_params;
    }
};


template<>
class LexicalCast<std::string, FilterConfs>
{
public:
    FilterConfs operator()(const std::string& str) {
        FilterConfs conf;
        YAML::Node node = YAML::Load(str);
        conf.name = node["name"].as<std::string>();
        conf.cls_name = node["class"].as<std::string>();
        conf.description = node["description"].as<std::string>();
        if (node["path"].IsDefined()) {
            for (auto path : node["path"]) { conf.url_patterns.push_back(path.as<std::string>()); }
        }
        if (node["init_params"].IsDefined()) {
            for (auto param : node["init_params"]) {
                conf.init_params.insert(
                    std::make_pair(param.first.as<std::string>(), param.second.as<std::string>()));
            }
        }
        return conf;
    }
};

template<>
class LexicalCast<FilterConfs, std::string>
{
public:
    std::string operator()(const FilterConfs& conf) {
        YAML::Node node;
        std::stringstream ss;
        node["name"] = conf.name;
        node["class"] = conf.cls_name;
        node["description"] = conf.description;
        for (auto& path : conf.url_patterns) { node["path"].push_back(path); }
        for (auto& param : conf.init_params) { node["init_params"][param.first] = param.second; }

        ss << node;
        return ss.str();
    }
};

FilterChain::Ptr findFilterChain(const std::string& path);
void addFilterChain(const std::string& url_pattern, FilterChain::Ptr filter_chain);

}   // namespace pico

#endif
