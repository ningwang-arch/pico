#ifndef __PICO_DAEMON_H__
#define __PICO_DAEMON_H__

#include <functional>
#include <sstream>
#include <string>
#include <unistd.h>

#include "singleton.h"
#include "util.h"

namespace pico {

struct ProcessInfo
{
    pid_t parent_id;
    pid_t main_id;

    uint64_t parent_start_time;
    uint64_t main_start_time;

    std::string to_string() {
        std::stringstream ss;

        ss << "[ProcessInfo parent_id=" << parent_id << " main_id=" << main_id
           << " parent_start_time=" << pico::Time2Str(parent_start_time)
           << " main_start_time=" << pico::Time2Str(main_start_time) << "]";

        return ss.str();
    }
};

typedef Singleton<ProcessInfo> ProcessMgr;

int start_daemon(int argc, char* argv[], std::function<int(int argc, char** argv)> main_cb,
                 bool is_daemon);

}   // namespace pico
#endif   // __PICO_DAEMON_H__