#include "filter.h"

namespace pico {
FilterChain::FilterChain() {}

FilterChain::FilterChain(const std::vector<FilterConfig::Ptr>& filters)
    : m_filters(filters) {
    m_size = filters.size();
}

FilterChain::FilterChain(const FilterChain& other) {
    m_filters = other.m_filters;
    m_size = other.m_size;
}

void FilterChain::doFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response) {
    internalDoFilter(request, response);
}

void FilterChain::internalDoFilter(const HttpRequest::Ptr& request, HttpResponse::Ptr& response) {
    if (m_index < m_size) {
        FilterConfig::Ptr filter_config = m_filters[m_index++];
        Filter::Ptr filter = filter_config->getFilter();
        filter->init(filter_config);
        filter->doFilter(request, response, shared_from_this());
    }
}

void FilterChain::addFilter(const FilterConfig::Ptr& filter_config) {
    m_filters.push_back(filter_config);
    m_size++;
}


FilterChain::~FilterChain() {}

}   // namespace pico