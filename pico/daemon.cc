#include "daemon.h"

#include <sys/wait.h>

#include "logging.h"

namespace pico {
const int restart_interval = 5;

int real_start(int argc, char* argv[], std::function<int(int argc, char** argv)> cb) {
    ProcessMgr::getInstance()->main_id = getpid();
    ProcessMgr::getInstance()->main_start_time = time(0);

    return cb(argc, argv);
}

int real_daemon(int argc, char* argv[], std::function<int(int argc, char** argv)> cb) {
    if (daemon(1, 0)) {
        LOG_ERROR("daemon error");
        return -1;
    }
    ProcessMgr::getInstance()->parent_id = getpid();
    ProcessMgr::getInstance()->parent_start_time = time(0);
    while (true) {
        pid_t pid = fork();
        if (pid == 0) {
            ProcessMgr::getInstance()->main_id = getpid();
            ProcessMgr::getInstance()->main_start_time = time(0);
            return real_start(argc, argv, cb);
        }
        else if (pid < 0) {
            LOG_ERROR("fork() failed");
            return -1;
        }
        else {
            int status = 0;
            waitpid(pid, &status, 0);
            if (status) {
                if (status == 9) {
                    LOG_DEBUG("child killed, pid = %d", pid);
                    break;
                }
                else {
                    LOG_ERROR("child crahsed, pid = %d", pid);
                }
            }
            else {
                LOG_INFO("child finished, pid = %d", pid);
                break;
            }
            sleep(restart_interval);
        }
    }
    return 0;
}


int start_daemon(int argc, char* argv[], std::function<int(int argc, char** argv)> cb,
                 bool is_daemon) {
    if (!is_daemon) {
        ProcessMgr::getInstance()->parent_id = getpid();
        ProcessMgr::getInstance()->parent_start_time = time(0);
        return real_start(argc, argv, cb);
    }
    return real_daemon(argc, argv, cb);
}


}   // namespace pico