#include "filter.h"

namespace pico {
FilterChain::FilterChain() {}

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
        filter->doFilter(request, response, shared_from_this());
    }
    else {
        m_servlet->service(request, response);
    }
}

void FilterChain::addFilter(const FilterConfig::Ptr& filter_config) {
    m_filters.push_back(filter_config);
    m_size++;
}


FilterChain::~FilterChain() {}

}   // namespace pico