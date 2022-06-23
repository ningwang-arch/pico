#include <sqlite3.h>
#include <stdio.h>
#include <string>

#include "pico/env.h"
#include "pico/iomanager.h"
#include "pico/mapper/common/mapper.hpp"
#include "pico/mapper/entity/entity_wrapper.h"

void test() {
    sqlite3* db;
    int rc = sqlite3_open("abc.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    std::string sql = "SELECT * FROM user";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        int type = sqlite3_column_type(stmt, 2);
        const char* name = (const char*)sqlite3_column_text(stmt, 1);
        const char* date = (const char*)sqlite3_column_text(stmt, 2);
        printf("%d %s %d %s\n", id, name, type, date);
    }
}

struct user
{
    int id;
    std::string name;
    std::string create_time;
};

ResultMap(EntityMap(user, "user"), PropertyMap(id, pico::ColumnType::Id), PropertyMap(name),
          PropertyMap(create_time));

void test_select() {
    pico::Mapper<user> mapper;
    mapper.use("sql_2");
    auto result = mapper.selectAll();
    for (auto& item : result) {
        printf("%d %s %s\n", item.id, item.name.c_str(), item.create_time.c_str());
    }
}

void test_insert() {
    pico::Mapper<user> mapper;
    mapper.use("sql_2");
    user user;
    user.name = "test";
    user.create_time = "2018-01-01 00:00:00";
    int ret = mapper.insert(user);
    printf("ret: %d\n", ret);
}

int main(int argc, char* argv[]) {
    pico::EnvManager::getInstance()->init(argc, argv);
    pico::Config::LoadFromConfDir(pico::EnvManager::getInstance()->getConfigPath());
    // test();
    // test_select();
    test_insert();
    return 0;
}
