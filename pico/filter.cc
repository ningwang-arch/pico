#include "filter.h"

#include <fnmatch.h>
#include <mutex>
namespace pico {

//  /abc/def/*  filter3 + /abc/*
//  /abc/*      filter2 + /*
//  /def        filter4 + /*
//  /*          filter1
static std::map<std::string, FilterChain::Ptr, InsertByLength> g_filter_chains = {};

static std::mutex g_mutex;

FilterChain::FilterChain() {}

FilterChain::FilterChain(const FilterChain& other)
    : m_filters(other.m_filters)
    , m_size(other.m_size) {}

void FilterChain::doFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response) {
    internalDoFilter(request, response);
}

void FilterChain::internalDoFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response) {
    if (m_index < m_size) {
        FilterConfig::Ptr filter_config = m_filters[m_index++];
        Filter::Ptr filter = filter_config->getFilter();
        filter->doFilter(request, response, shared_from_this());
    }
    else {
        m_servlet->service(request, response);
    }
}

void FilterChain::addFilter(const FilterConfig::Ptr& filter_config) {
    // check if the filter is already in the chain
    auto it =
        std::find_if(m_filters.begin(), m_filters.end(), [&](const FilterConfig::Ptr& filter) {
            return filter->getFilter() == filter_config->getFilter();
        });
    if (it == m_filters.end()) {
        m_filters.push_back(filter_config);
        m_size++;
    }
}





FilterChain::~FilterChain() {}

FilterChain::Ptr findFilterChain(const std::string& path) {

    for (auto& filter_chain : g_filter_chains) {
        if (fnmatch(filter_chain.first.c_str(), path.c_str(), 0) == 0) {
            return filter_chain.second;
        }
    }
    return nullptr;
}

void addFilterChain(const std::string& url_pattern, FilterChain::Ptr filter_chain) {
    std::lock_guard<std::mutex> lock(g_mutex);
    auto chain = findFilterChain(url_pattern);
    if (chain) {
        if (chain != filter_chain) {
            auto filters = chain->getFilters();
            for (auto& filter : filters) {
                filter_chain->addFilter(filter);
            }
        }
    }
    g_filter_chains[url_pattern] = filter_chain;
}

}   // namespace pico