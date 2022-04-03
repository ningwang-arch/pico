#ifndef __PICO_URI_H__
#define __PICO_URI_H__

#include <memory>
#include <string>

#include "address.h"

/*

         foo://example.com:8042/over/there?name=ferret#nose
         \_/   \______________/\_________/ \_________/ \__/
          |           |            |            |        |
       scheme     authority       path        query   fragment
          |   _____________________|__
         / \ /                        \
         urn:example:animal:ferret:nose

*/

namespace pico {
class Uri
{
public:
    typedef std::shared_ptr<Uri> Ptr;

    static Uri::Ptr Create(const std::string& uristr);
    Uri();

    // getters
    const std::string& getScheme() const { return m_scheme; }
    const std::string& getHost() const { return m_host; }
    int32_t getPort() const;
    const std::string& getPath() const;
    const std::string& getQuery() const { return m_query; }
    const std::string& getFragment() const { return m_fragment; }
    const std::string& getUserInfo() const { return m_userInfo; }

    // setters
    void setScheme(const std::string& scheme) { m_scheme = scheme; }
    void setHost(const std::string& host) { m_host = host; }
    void setPort(int port) { m_port = port; }
    void setPath(const std::string& path) { m_path = path; }
    void setQuery(const std::string& query) { m_query = query; }
    void setFragment(const std::string& fragment) { m_fragment = fragment; }
    void setUserInfo(const std::string& userInfo) { m_userInfo = userInfo; }

    std::string toString() const;

    Address::Ptr getAddress() const;

private:
    bool isDefaultPort() const;

private:
    std::string m_scheme;
    std::string m_host;
    int32_t m_port;
    std::string m_path;
    std::string m_query;
    std::string m_fragment;
    std::string m_userInfo;
};
}   // namespace pico

#endif