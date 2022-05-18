#include "date.h"

#include <iostream>
#include <sys/time.h>

#include "util.h"

namespace pico {

Date::Date() {
    m_tm = {};
    time_t t = time(NULL);
    m_tm = *localtime(&t);
}

Date::Date(const std::string& date, std::string format) {
    // init tm
    m_tm = {};
    if (date.empty()) { Date(); }
    if (format.empty()) { format = "%Y-%m-%d %H:%M:%S"; }
    strptime(date.data(), format.data(), &m_tm);
}

Date::Date(time_t timestamp) {
    m_tm = *localtime(&timestamp);
}


std::string Date::now(const std::string& format) {
    time_t t = time(NULL);
    return Time2Str(t, format.data());
}

std::string Date::to_string() {
    time_t t = mktime(&m_tm);
    return Time2Str(t, m_format.data());
}

bool Date::isAfter(Date& other) {
    return mktime(&m_tm) > mktime(&other.m_tm);
}

bool Date::isBefore(Date& other) {
    return mktime(&m_tm) < mktime(&other.m_tm);
}

bool Date::isEqual(Date& other) {
    return mktime(&m_tm) == mktime(&other.m_tm);
}

Date Date::operator+(const time_t seconds) {
    time_t t = mktime(&m_tm);
    t += seconds;
    return Date(t);
}

Date Date::operator-(const time_t seconds) {
    time_t t = mktime(&m_tm);
    t -= seconds;
    return Date(t);
}

}   // namespace pico