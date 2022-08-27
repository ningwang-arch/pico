#ifndef __PICO_REDIS_H__
#define __PICO_REDIS_H__

#include <assert.h>
#include <condition_variable>
#include <hiredis/hiredis.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <yaml-cpp/yaml.h>


#include "config.h"
#include "serialize.hpp"
#include "singleton.h"


namespace pico {

struct RedisInfo
{
    std::string host = "";
    int port = 0;
    int db = 0;
    std::string passwd = "";
    int timeout = 0;
};

template<>
class LexicalCast<std::string, RedisInfo>
{
public:
    RedisInfo operator()(const std::string& str) {
        YAML::Node node = YAML::Load(str);
        RedisInfo info;
        info.host = node["host"].as<std::string>(info.host);
        info.port = node["port"].as<int>(info.port);
        info.db = node["database"].as<int>(info.db);
        info.passwd = node["password"].as<std::string>(info.passwd);
        info.timeout = node["timeout"].as<int>(info.timeout);
        return info;
    }
};

template<>
class LexicalCast<RedisInfo, std::string>
{
public:
    std::string operator()(const RedisInfo& info) {
        YAML::Node node;
        node["host"] = info.host;
        node["port"] = info.port;
        node["database"] = info.db;
        node["password"] = info.passwd;
        node["timeout"] = info.timeout;
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};


static ConfigVar<RedisInfo>::Ptr g_redis_info =
    Config::Lookup<RedisInfo>("redis.config", RedisInfo(), "redis config");

enum RedisType
{
    REDIS_STRING = 0,
    REDIS_LIST = 1,
    REDIS_SET = 2,
    REDIS_ZSET = 3,
    REDIS_HASH = 4
};

enum RedisStatus
{

    REDIS_STATUS_OK = 0,   // everything went well
    CONNECT_FAIL = 1,      // can't connect to redis
    AUTH_FAIL = 2,         // auth fail
    SELECT_FAIL = 3,       // select db fail
    CONTEXT_ERROR = 4,     // can't create context
    REPLY_ERROR = 5,       // can't get reply
    COMMAND_ERROR = 6,     // can't execute command
    NIL_ERROR = 7,         // nil
    BUFFER_ERROR = 8,      // buffer error
};

template<typename T>
struct RedisResult
{
    RedisStatus status = REDIS_STATUS_OK;
    std::string error = "";
    T data;
};

class RedisConnection
{
public:
    RedisConnection() {
        _context = NULL;
        _reply = NULL;
    }

    ~RedisConnection() {
        if (_context) {
            redisFree(_context);
        }
        if (_reply) {
            freeReplyObject(_reply);
        }
    }

    RedisStatus connect(const char* host, int port, int db = 0, int timeout = 0,
                        const char* password = NULL) {
        _context = redisConnect(host, port);
        if (_context == NULL || _context->err) {
            if (_context) {
                redisFree(_context);
                _context = NULL;
                return CONNECT_FAIL;
            }
        }
        if (timeout > 0) {
            timeval tv;
            tv.tv_sec = timeout;
            tv.tv_usec = 0;
            redisSetTimeout(_context, tv);
        }
        if (password != NULL && strcmp(password, "") != 0) {
            redisReply* reply = (redisReply*)redisCommand(_context, "AUTH %s", password);
            if (reply == NULL) {
                redisFree(_context);
                _context = NULL;
                return AUTH_FAIL;
            }
            freeReplyObject(reply);
        }
        redisReply* reply = (redisReply*)redisCommand(_context, "SELECT %d", db);
        if (reply == NULL) {
            redisFree(_context);
            _context = NULL;
            return SELECT_FAIL;
        }
        return REDIS_STATUS_OK;
    }

    int disconnect() {
        if (_context) {
            redisFree(_context);
            _context = NULL;
        }
        return REDIS_STATUS_OK;
    }


    template<typename T>
    RedisStatus set(const char* key, const T& value) {
        std::string str = pico::serialize(value);
        _reply = (redisReply*)redisCommand(_context, "SET %s %s", key, str.c_str());
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    template<typename T>
    RedisResult<T> get(const char* key) {
        RedisResult<T> result;
        _reply = (redisReply*)redisCommand(_context, "GET %s", key);
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data = pico::deserialize<T>(std::string(_reply->str));
        return result;
    }

    RedisStatus del(const char* key) {
        _reply = (redisReply*)redisCommand(_context, "DEL %s", key);
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    template<typename T>
    RedisStatus hset(const char* key, const char* field, const T& value) {
        std::string str = pico::serialize(value);
        _reply = (redisReply*)redisCommand(_context, "HSET %s %s %s", key, field, str.c_str());
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    template<typename T>
    RedisResult<T> hget(const char* key, const char* field) {
        RedisResult<T> result;
        _reply = (redisReply*)redisCommand(_context, "HGET %s %s", key, field);
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data = pico::deserialize<T>(std::string(_reply->str));
        return result;
    }

    RedisStatus hdel(const char* key, const char* field) {
        _reply = (redisReply*)redisCommand(_context, "HDEL %s %s", key, field);
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    int hlen(const char* key) {
        _reply = (redisReply*)redisCommand(_context, "HLEN %s", key);
        if (_reply == NULL) {
            return -1;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return -1;
        }
        return _reply->integer;
    }

    RedisResult<std::vector<std::string>> hkeys(const char* key) {
        RedisResult<std::vector<std::string>> result;
        _reply = (redisReply*)redisCommand(_context, "HKEYS %s", key);
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data.resize(_reply->elements);
        for (size_t i = 0; i < _reply->elements; i++) {
            result.data[i] = _reply->element[i]->str;
        }
        return result;
    }

    RedisResult<std::vector<std::string>> hvals(const char* key) {
        RedisResult<std::vector<std::string>> result;
        _reply = (redisReply*)redisCommand(_context, "HVALS %s", key);
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data.resize(_reply->elements);
        for (size_t i = 0; i < _reply->elements; i++) {
            result.data[i] = _reply->element[i]->str;
        }
        return result;
    }

    RedisResult<std::vector<std::string>> hgetall(const char* key) {
        RedisResult<std::vector<std::string>> result;
        _reply = (redisReply*)redisCommand(_context, "HGETALL %s", key);
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data.resize(_reply->elements);
        for (size_t i = 0; i < _reply->elements; i++) {
            result.data[i] = _reply->element[i]->str;
        }
        return result;
    }

    RedisStatus hincrby(const char* key, const char* field, int incr) {
        _reply = (redisReply*)redisCommand(_context, "HINCRBY %s %s %d", key, field, incr);
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    RedisStatus hincrbyfloat(const char* key, const char* field, float incr) {
        _reply = (redisReply*)redisCommand(_context, "HINCRBYFLOAT %s %s %f", key, field, incr);
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    RedisStatus hmset(const std::map<std::string /*key*/, std::string /*value*/> data) {
        std::string cmd = "HMSET ";
        for (auto it = data.begin(); it != data.end(); it++) {
            cmd += it->first;
            cmd += " ";
            cmd += it->second;
            cmd += " ";
        }
        _reply = (redisReply*)redisCommand(_context, cmd.c_str());
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    RedisResult<std::vector<std::string>> hmget(const char* key,
                                                const std::vector<std::string>& fields) {
        RedisResult<std::vector<std::string>> result;
        std::string cmd = "HMGET ";
        cmd += key;
        cmd += " ";
        for (auto it = fields.begin(); it != fields.end(); it++) {
            cmd += *it;
            cmd += " ";
        }
        _reply = (redisReply*)redisCommand(_context, cmd.c_str());
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data.resize(_reply->elements);
        for (size_t i = 0; i < _reply->elements; i++) {
            result.data[i] = _reply->element[i]->str;
        }
        return result;
    }

    RedisStatus hdelall(const char* key) {
        _reply = (redisReply*)redisCommand(_context, "HDEL %s *", key);
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    template<typename T>
    RedisStatus sadd(const char* key, const T& member) {
        std::string str = pico::serialize(member);
        _reply = (redisReply*)redisCommand(_context, "SADD %s %s", key, str.c_str());
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    template<typename T>
    RedisStatus srem(const char* key, const T& member) {
        std::string str = pico::serialize(member);
        _reply = (redisReply*)redisCommand(_context, "SREM %s %s", key, str.c_str());
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    RedisResult<std::vector<std::string>> smembers(const char* key) {
        RedisResult<std::vector<std::string>> result;
        _reply = (redisReply*)redisCommand(_context, "SMEMBERS %s", key);
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data.resize(_reply->elements);
        for (size_t i = 0; i < _reply->elements; i++) {
            result.data[i] = _reply->element[i]->str;
        }
        return result;
    }

    template<typename T>
    bool sismember(const char* key, const T& member) {
        std::string str = pico::serialize(member);
        _reply = (redisReply*)redisCommand(_context, "SISMEMBER %s %s", key, str.c_str());
        if (_reply == NULL) {
            return false;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return false;
        }
        return _reply->integer == 1;
    }
    int scard(const char* key) {
        _reply = (redisReply*)redisCommand(_context, "SCARD %s", key);
        if (_reply == NULL) {
            return 0;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return 0;
        }
        return _reply->integer;
    }
    RedisResult<std::vector<std::string>> sinter(std::vector<std::string> keys) {
        RedisResult<std::vector<std::string>> result;
        std::string cmd = "SINTER ";
        for (auto it = keys.begin(); it != keys.end(); it++) {
            cmd += *it;
            cmd += " ";
        }
        _reply = (redisReply*)redisCommand(_context, cmd.c_str());
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data.resize(_reply->elements);
        for (size_t i = 0; i < _reply->elements; i++) {
            result.data[i] = _reply->element[i]->str;
        }
        return result;
    }
    RedisStatus sinterstore(const char* destination, const std::vector<std::string>& keys) {
        std::string cmd = "SINTERSTORE ";
        cmd += destination;
        cmd += " ";
        for (auto it = keys.begin(); it != keys.end(); it++) {
            cmd += *it;
            cmd += " ";
        }
        _reply = (redisReply*)redisCommand(_context, cmd.c_str());
        if (_reply == NULL) {
            return COMMAND_ERROR;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return REPLY_ERROR;
        }
        return REDIS_STATUS_OK;
    }

    RedisResult<std::vector<std::string>> sunion(std::vector<std::string> keys) {
        RedisResult<std::vector<std::string>> result;
        std::string cmd = "SUNION ";
        for (auto it = keys.begin(); it != keys.end(); it++) {
            cmd += *it;
            cmd += " ";
        }
        _reply = (redisReply*)redisCommand(_context, cmd.c_str());
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data.resize(_reply->elements);
        for (size_t i = 0; i < _reply->elements; i++) {
            result.data[i] = _reply->element[i]->str;
        }
        return result;
    }

    int sunionstore(const char* destination, const std::vector<std::string>& keys) {
        std::string cmd = "SUNIONSTORE ";
        cmd += destination;
        cmd += " ";
        for (auto it = keys.begin(); it != keys.end(); it++) {
            cmd += *it;
            cmd += " ";
        }
        _reply = (redisReply*)redisCommand(_context, cmd.c_str());
        if (_reply == NULL) {
            return 0;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return 0;
        }
        return _reply->integer;
    }

    RedisResult<std::vector<std::string>> sdiff(const std::vector<std::string>& keys) {
        RedisResult<std::vector<std::string>> result;
        std::string cmd = "SDIFF ";
        for (auto it = keys.begin(); it != keys.end(); it++) {
            cmd += *it;
            cmd += " ";
        }
        _reply = (redisReply*)redisCommand(_context, cmd.c_str());
        if (_reply == NULL) {
            result.status = COMMAND_ERROR;
            result.error = "command error";
            return result;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            result.status = REPLY_ERROR;
            result.error = _reply->str;
            return result;
        }
        if (_reply->type == REDIS_REPLY_NIL) {
            result.status = NIL_ERROR;
            result.error = "nil";
            return result;
        }
        result.data.resize(_reply->elements);
        for (size_t i = 0; i < _reply->elements; i++) {
            result.data[i] = _reply->element[i]->str;
        }
        return result;
    }

    int sdiffstore(const char* destination, const std::vector<std::string>& keys) {
        std::string cmd = "SDIFFSTORE ";
        cmd += destination;
        cmd += " ";
        for (auto it = keys.begin(); it != keys.end(); it++) {
            cmd += *it;
            cmd += " ";
        }
        _reply = (redisReply*)redisCommand(_context, cmd.c_str());
        if (_reply == NULL) {
            return 0;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return 0;
        }
        return _reply->integer;
    }


    template<typename T>
    int smove(const char* source, const char* destination, const T& member) {
        std::string str = pico::serialize(member);
        _reply =
            (redisReply*)redisCommand(_context, "SMOVE %s %s %s", source, destination, str.c_str());
        if (_reply == NULL) {
            return 0;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return 0;
        }
        return _reply->integer;
    }

    template<typename T>
    int sismember(const char* key, const T& member) {
        std::string str = pico::serialize(member);
        _reply = (redisReply*)redisCommand(_context, "SISMEMBER %s %s", key, str.c_str());
        if (_reply == NULL) {
            return 0;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return 0;
        }
        return _reply->integer;
    }

    template<typename T>
    int srem(const char* key, const T& member) {
        std::string str = pico::serialize(member);
        _reply = (redisReply*)redisCommand(_context, "SREM %s %s", key, str.c_str());
        if (_reply == NULL) {
            return 0;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return 0;
        }
        return _reply->integer;
    }

    int sremall(const char* key) {
        _reply = (redisReply*)redisCommand(_context, "SREM %s", key);
        if (_reply == NULL) {
            return 0;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return 0;
        }
        return _reply->integer;
    }

    redisReply* execute(const char* cmd) {
        _reply = (redisReply*)redisCommand(_context, cmd);
        if (_reply == NULL) {
            return NULL;
        }
        if (_reply->type == REDIS_REPLY_ERROR) {
            return NULL;
        }
        return _reply;
    }



private:
    redisContext* _context;
    redisReply* _reply;
};


class RedisManager : public Singleton<RedisManager>
{
public:
    RedisManager() {
        for (int i = 0; i < _min_conn_num; i++) {
            RedisConnection* conn = createConnection();
            _connections.push(conn);
        }
    }

    std::shared_ptr<RedisConnection> getConnection() {
        std::unique_lock<std::mutex> lock(_mutex);
        if (!_connections.empty()) {
            return popConnection();
        }
        if (_open_conn_num < _max_conn_num) {
            return std::shared_ptr<RedisConnection>(
                createConnection(), [this](RedisConnection* conn) { releaseConnection(conn); });
        }
        if (!_cond.wait_for(lock, std::chrono::seconds(_idle_time), [this]() {
                return !_connections.empty();
            })) {
            return nullptr;
        }

        return popConnection();
    }

    ~RedisManager() {
        while (!_connections.empty()) {
            RedisConnection* conn = _connections.front();
            _connections.pop();
            delete conn;
        }
    }

private:
    RedisConnection* createConnection() {
        RedisConnection* conn = new RedisConnection();
        auto redisInfo = g_redis_info->getValue();
        conn->connect(redisInfo.host.data(),
                      redisInfo.port,
                      redisInfo.db,
                      redisInfo.timeout,
                      redisInfo.passwd.data());
        _open_conn_num++;
        return conn;
    }

    void releaseConnection(RedisConnection* conn) {
        if (conn) {
            std::unique_lock<std::mutex> lock(_mutex);
            _connections.push(conn);
            _cond.notify_one();
        }
    }

    std::shared_ptr<RedisConnection> popConnection() {
        if (_connections.empty()) {
            return nullptr;
        }
        auto conn = _connections.front();
        _connections.pop();
        return std::shared_ptr<RedisConnection>(
            conn, [this](RedisConnection* conn) { releaseConnection(conn); });
    }

private:
    std::queue<RedisConnection*> _connections;
    std::mutex _mutex;
    std::condition_variable _cond;

    int _min_conn_num = 10;
    int _max_conn_num = 20;

    int _idle_time = 60;


    int _open_conn_num = 0;
};

}   // namespace pico

#endif   // __PICO_REDIS_H__