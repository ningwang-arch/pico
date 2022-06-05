#ifndef __PICO_MAPPER_HELPER_QUERY_HELPER_HPP__
#define __PICO_MAPPER_HELPER_QUERY_HELPER_HPP__
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "../entity/base.hpp"
#include "../entity/entity_column.hpp"
#include "../entity/iterable.hpp"
#include "../sql/connection.h"
#include "../sql/connection_pool.h"
#include "../sql/sql_option.h"
#include "entity_helper.hpp"
#include "pico/logging.h"


namespace pico {
static ConfigVar<std::unordered_map<std::string, SQLOption>>::Ptr g_sql_options =
    Config::Lookup<std::unordered_map<std::string, SQLOption>>(
        "dbs", std::unordered_map<std::string, SQLOption>(), "sql_options");

class QueryHelper
{
public:
    template<typename Entity>
    static void select(const std::string& sql, const Iterable& args, std::vector<Entity>& results,
                       std::string sql_conf = "") {
        auto conn = getConnection(sql_conf);
        if (!conn) return;
        if (!prepare(conn, sql, args)) { return; }


        std::unordered_map<int, int> id_index_map;
        Base<Entity> tmp;
        std::unordered_map<std::string, EntityColumn> col_map = tmp.getColumnAliasMap();
        std::vector<EntityColumn> join_cols = tmp.getJoinEntityColumns();
        std::shared_ptr<EntityColumn> key_col = tmp.getKeyEntityColumn();
        auto container_iter =
            std::find_if(join_cols.begin(), join_cols.end(), [](EntityColumn& entity_col) {
                return entity_col.container();
            });
        int cnt = 0;
        while (conn->next()) {
            cnt++;
            int id = 0;
            std::set<std::string> null_join_property_set;
            for (size_t i = 0; i < conn->getRecords().size(); i++) {
                auto col = col_map[conn->getRecords()[i]];
                Object value = conn->value(i);
                if (value.isNull()) {
                    auto join_property_iter = std::find_if(
                        join_cols.begin(), join_cols.end(), [&col](EntityColumn& entity_col) {
                            return entity_col.getJoinProperty() == col.getJoinProperty();
                        });
                    if (join_property_iter != join_cols.end()) {
                        null_join_property_set.insert(join_property_iter->getProperty());
                    }
                }
                col.bindValue2EntityField(conn->value(i));
                if (key_col != nullptr &&
                    col.getColumnWithTableAlias() == key_col->getColumnWithTableAlias()) {
                    id = conn->value(i).getValue<int>();
                }
            }
            std::shared_ptr<Entity> entity_result = tmp.getEntity();
            for (auto& null_property : null_join_property_set) {
                EntityHelper::clearPropertyValues(null_property, entity_result.get());
            }
            if (container_iter == std::end(join_cols)) {
                results.push_back(*entity_result);
                continue;
            }
            if (id != 0 && id_index_map.count(id) <= 0) {
                results.push_back(*entity_result);
                id_index_map.insert({id, results.size() - 1});
            }
            else if (id_index_map.count(id) > 0) {
                auto& entity = results[id_index_map[id]];
                for (const auto& join_col : join_cols) {
                    EntityHelper::appendPropertyValues(
                        join_col.getProperty(), tmp.getEntity().get(), &entity);
                }
            }
        }
    }

    static void select(const std::string& sql, const Iterable& args,
                       std::vector<std::vector<Object>>& results, std::string sql_conf = "") {
        auto conn = getConnection(sql_conf);
        if (!conn) return;
        if (!prepare(conn, sql, args)) return;

        while (conn->next()) {
            std::vector<Object> record;
            for (size_t i = 0; i < conn->getRecords().size(); ++i) {
                record.emplace_back(conn->value(i));
            }
            results.emplace_back(record);
        }
    }

    static std::pair<int, int> execute(const std::string& sql, const Iterable& args,
                                       std::string sql_conf = "") {
        auto conn = getConnection(sql_conf);
        if (!conn) return {};
        if (!prepare(conn, sql, args)) return {};
        return {conn->getLastAffectedRows(), conn->getLastInsertId()};
    }

private:
    static std::shared_ptr<Connection> getConnection(std::string sql_conf = "") {
        if (sql_conf.empty()) {
            auto sql_options = g_sql_options->getValue();
            if (sql_options.size() != 1) {
                LOG_ERROR("No database selected");
                return nullptr;
            }
            sql_conf = sql_options.begin()->first;
        }
        auto conn = ConnectionManager::getInstance()->getConnection(sql_conf);
        return conn;
    }

    static bool prepare(const std::shared_ptr<Connection>& conn, const std::string& sql,
                        const Iterable& args) {

        if (!conn->prepare(sql)) { return false; }
        for (size_t i = 0; i < args.size(); i++) { conn->bindValue((int)i, args[i]); }
        return conn->execute();
    }
};

}   // namespace pico


#endif   // !__PICO_MAPPER_HELPER_QUERY_HELPER_HPP__
