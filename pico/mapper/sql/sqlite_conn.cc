#include "sqlite_conn.h"


namespace pico {
void SQLiteConnection::close() {
    if (_conn) {
        sqlite3_close(_conn);
        _conn = nullptr;
    }

    if (_stmt) {
        sqlite3_finalize(_stmt);
        _stmt = nullptr;
    }
}

bool SQLiteConnection::ping() {
    return true;
}

void SQLiteConnection::bindInt(int index, const Object& value) {
    sqlite3_bind_int(_stmt, index, value.getValue<int>());
}

void SQLiteConnection::bindUInt(int index, const Object& value) {
    sqlite3_bind_int64(_stmt, index, value.getValue<int>());
}

void SQLiteConnection::bindTime(int index, const Object& value) {
    sqlite3_bind_int64(_stmt, index, value.getValue<std::time_t>());
}

void SQLiteConnection::bindString(int index, const Object& value) {
    // https://stackoverflow.com/a/1229265
    sqlite3_bind_text(_stmt,
                      index,
                      value.getValue<std::string>().c_str(),
                      value.getValue<std::string>().size(),
                      SQLITE_TRANSIENT);
}

int SQLiteConnection::getLastAffectedRows() {

    std::string sql = "SELECT changes()";
    if (sqlite3_prepare_v2(_conn, sql.c_str(), -1, &_stmt, nullptr) != SQLITE_OK) { return -1; }
    if (sqlite3_step(_stmt) != SQLITE_ROW) { return -1; }
    return sqlite3_column_int(_stmt, 0);
}

int SQLiteConnection::getLastInsertId() {
    return sqlite3_last_insert_rowid(_conn);
}

bool SQLiteConnection::next() {
    return ++_current_row_index < (int)_values.size();
}

bool SQLiteConnection::connect() {

    if (_conn) { return true; }

    int ret = sqlite3_open(_option.database.c_str(), &_conn);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_open failed: {}", sqlite3_errmsg(_conn));
        return false;
    }
    return true;
}

bool SQLiteConnection::begin() {

    if (!connect()) { return false; }

    int ret = sqlite3_exec(_conn, "BEGIN", nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_exec failed: {}", sqlite3_errmsg(_conn));
        return false;
    }
    return true;
}

bool SQLiteConnection::commit() {

    if (!connect()) { return false; }

    int ret = sqlite3_exec(_conn, "COMMIT", nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_exec failed: {}", sqlite3_errmsg(_conn));
        return false;
    }
    return true;
}

bool SQLiteConnection::rollback() {

    if (!connect()) { return false; }

    int ret = sqlite3_exec(_conn, "ROLLBACK", nullptr, nullptr, nullptr);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_exec failed: {}", sqlite3_errmsg(_conn));
        return false;
    }
    return true;
}

bool SQLiteConnection::prepare(const std::string& sql) {

    if (!connect()) { return false; }

    int ret = sqlite3_prepare_v2(_conn, sql.c_str(), -1, &_stmt, nullptr);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_prepare_v2 failed: %s", sqlite3_errmsg(_conn));
        return false;
    }
    return true;
}

void SQLiteConnection::bindValue(int index, const Object& value) {
    auto type = value.getType();
    auto it = _bind_map.find(type);
    if (it == _bind_map.end()) {
        LOG_ERROR("bind type not found: {}", type);
        return;
    }
    // https://stackoverflow.com/a/31747742
    // sqlite stmt bind index starts from 1
    it->second(index + 1, value);
}
bool SQLiteConnection::execute() {

    if (!connect()) { return false; }

    int ret = sqlite3_step(_stmt);
    // sqlite3_step returns SQLITE_ROW or SQLITE_DONE
    // SQLITE_ROW means there is a row to be retrieved
    // SQLITE_DONE means there is no row to be retrieved, usually used for insert/update/delete

    if (ret == SQLITE_DONE) { return true; }
    if (ret != SQLITE_ROW) {
        LOG_ERROR("sqlite3_step failed: {}", sqlite3_errmsg(_conn));
        return false;
    }

    std::vector<Object> values;
    do {
        int column_count = sqlite3_column_count(_stmt);
        _records.clear();
        values.clear();
        values.resize(column_count);

        for (int i = 0; i < column_count; ++i) {
            int type = sqlite3_column_type(_stmt, i);
            std::string column_name = sqlite3_column_name(_stmt, i);
            _records.emplace_back(column_name);
            switch (type) {
            case SQLITE_INTEGER: values[i] = Object(sqlite3_column_int(_stmt, i)); break;
            case SQLITE_TEXT:
            {
                const char* text = (const char*)sqlite3_column_text(_stmt, i);
                values[i] = Object(text);
                break;
            }
            case SQLITE_NULL: values[i] = Object(); break;
            default:
                const char* text = (const char*)sqlite3_column_text(_stmt, i);
                values[i] = Object(text);
                break;
            }
        }
        _values.emplace_back(values);
    } while (sqlite3_step(_stmt) == SQLITE_ROW);
    return true;
}

SQLiteConnection::~SQLiteConnection() {
    close();
}

}   // namespace pico