#ifndef __PICO_DATE_H__
#define __PICO_DATE_H__

#include <string>
#include <time.h>

namespace pico {

/**
 * @brief The Date class
 * @details
 *  Date class is a wrapper of struct tm.
 *  So it's accuracy is second.
 *  If you want to get more accurate time,
 *  you should rewrite this class.
 *  But please be careful to change the method function call.
 */
class Date
{
public:
    Date();
    Date(Date&& other) = default;
    Date(const Date& other) {
        m_tm = other.m_tm;
        m_format = other.m_format;
    }
    Date(const std::string& date, std::string format = "%Y-%m-%d %H:%M:%S");
    Date(time_t timestamp);

    Date& operator=(const Date& other) {
        this->m_tm = other.m_tm;
        this->m_format = other.m_format;
        return *this;
    }


    void setYear(int year) { m_tm.tm_year = year - 1900; }
    int getYear() const { return m_tm.tm_year + 1900; }

    void setMonth(int month) {
        if (month < 1 || month > 12) {
            return;
        }
        m_tm.tm_mon = month - 1;
    }
    int getMonth() const { return m_tm.tm_mon + 1; }

    void setDay(int day) {
        if (day < 1 || day > 31) {
            return;
        }
        m_tm.tm_mday = day;
    }
    int getDay() const { return m_tm.tm_mday; }

    void setHour(int hour) {
        if (hour < 0 || hour > 23) {
            return;
        }
        m_tm.tm_hour = hour;
    }
    int getHour() const { return m_tm.tm_hour; }

    void setMinute(int minute) {
        if (minute < 0 || minute > 59) {
            return;
        }
        m_tm.tm_min = minute;
    }
    int getMinute() const { return m_tm.tm_min; }

    void setSecond(int second) {
        if (second < 0 || second > 59) {
            return;
        }
        m_tm.tm_sec = second;
    }
    int getSecond() const { return m_tm.tm_sec; }

    void setWeekday(int weekday) {
        if (weekday < 0 || weekday > 6) {
            return;
        }
        m_tm.tm_wday = weekday;
    }
    int getWeekday() const { return m_tm.tm_wday; }

    void setFomat(const std::string& format) { m_format = format; }
    std::string getFomat() const { return m_format; }


    static std::string now(const std::string& format = "%Y-%m-%d %H:%M:%S");

    std::string to_string();

    bool isAfter(Date& other);
    bool isBefore(Date& other);
    bool isEqual(Date& other);

    Date operator+(const time_t seconds);
    Date operator-(const time_t seconds);


private:
    std::string format(const std::string& format = "%Y-%m-%d %H:%M:%S") const;

private:
    struct tm m_tm;
    std::string m_format = "%Y-%m-%d %H:%M:%S";
};

}   // namespace pico

#endif