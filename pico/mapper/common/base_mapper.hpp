#ifndef __PICO_MAPPER_COMMON_BASE_MAPPER_HPP__
#define __PICO_MAPPER_COMMON_BASE_MAPPER_HPP__
#include <algorithm>
#include <exception>
#include <memory>
#include <string>
#include <vector>

#include "../entity/base.hpp"
#include "../entity/criteria.hpp"
#include "../entity/entity_column.hpp"
#include "../entity/object.hpp"
#include "../helper/query_helper.hpp"
#include "../mapper_exception.h"

namespace pico {
template<typename T>
class BaseMapper
{
public:
    BaseMapper() = default;
    BaseMapper(const std::string& sql_conf)
        : m_sql_conf(std::move(sql_conf)) {}

    std::vector<T> select(const T& record) {
        Base<T> base;
        std::shared_ptr<Criteria> criteria = base.createCriteria();
        criteria->andEqualTo(record);
        return select(base);
    }

    std::vector<T> select(const Base<T>& base) {
        std::vector<T> results;
        auto select_ctx = base.getSelectContext();
        QueryHelper::select(select_ctx.first, select_ctx.second, results, m_sql_conf);
        return results;
    }

    T selectOne(const T& record) {
        auto results = select(record);
        if (results.size() > 1) {
            throw MapperException("select results have more than one record");
        }
        return results.empty() ? T{} : results.front();
    }

    T selectOne(const Base<T>& record) {
        auto results = select(record);
        if (results.size() > 1) {
            throw MapperException("select results have more than one record");
        }
        return results.empty() ? T{} : results.front();
    }
    std::vector<T> selectAll() {
        Base<T> base;
        return select(base);
    }

    int selectCount(const Base<T>& base) {
        std::vector<std::vector<Object>> results;
        auto select_count_ctx = base.getSelectCount();
        QueryHelper::select(select_count_ctx.first, select_count_ctx.second, results, m_sql_conf);
        return results.front().front().getValue<int>();
    }

    int selectCount(const T& record) {
        Base<T> base;
        std::shared_ptr<Criteria> criteria = base.createCriteria();
        criteria->andEqualTo(record);
        return selectCount(base);
    }

    T selectByPrimaryKey(const Object& key) {
        Base<T> base;
        std::shared_ptr<EntityColumn> key_col = base.getKeyEntityColumn();
        std::shared_ptr<Criteria> criteria = base.createCriteria();
        criteria->andEqualTo(key_col->getProperty(), key);
        return selectOne(base);
    }

    bool existsWithPrimaryKey(const Object& key) {
        Base<T> base;
        std::shared_ptr<EntityColumn> key_col = base.getKeyEntityColumn();
        std::shared_ptr<Criteria> criteria = base.createCriteria();
        criteria->andEqualTo(key_col->getProperty(), key);

        return selectCount(base) > 0;
    }

    int update(const T& record, const Base<T>& base) {
        auto update_ctx = base.getUpdateContext(record, false);
        return QueryHelper::execute(update_ctx.first, update_ctx.second, m_sql_conf).first;
    }
    int updateSelective(const T& record, const Base<T>& base) {
        auto update_ctx = base.getUpdateContext(record, true);
        return QueryHelper::execute(update_ctx.first, update_ctx.second, m_sql_conf).first;
    }

    int updateByPrimaryKey(const T& record) {
        Base<T> base;
        std::shared_ptr<EntityColumn> key_col = base.getKeyEntityColumn(&record);
        std::shared_ptr<Criteria> criteria = base.createCriteria();
        criteria->andEqualTo(key_col->getProperty(), key_col->getEntityFieldValue());
        return update(record, base);
    }

    int updateByPrimayKeySelective(const T& record) {
        Base<T> base;
        std::shared_ptr<EntityColumn> key_col = base.getKeyEntityColumn(record);
        std::shared_ptr<Criteria> criteria = base.createCriteria();
        criteria->andEqualTo(key_col->getProperty(), key_col->getEntityFieldValue());
        return updateSelective(record, base);
    }

    int insert(const T& record) {
        Base<T> base;
        std::pair<std::string, std::vector<Object>> insert_ctx =
            base.getInsertContext(record, false);
        return QueryHelper::execute(insert_ctx.first, insert_ctx.second, m_sql_conf).second;
    }

    int insertSelective(const T& record) {
        Base<T> base;
        std::pair<std::string, std::vector<Object>> insert_ctx =
            base.getInsertContext(record, true);
        return QueryHelper::execute(insert_ctx.first, insert_ctx.second, m_sql_conf).second;
    }

    int deleteBy(const Base<T>& base) {
        auto delete_ctx = base.getDeleteContext();
        return QueryHelper::execute(delete_ctx.first, delete_ctx.second, m_sql_conf).first;
    }

    int deleteBy(const T& record) {
        Base<T> base;
        std::shared_ptr<Criteria> criteria = base.createCriteria();
        criteria->andEqualTo(record);
        return deleteBy(base);
    }

    int deleteByPrimaryKey(const Object& key) {
        Base<T> base;
        std::shared_ptr<EntityColumn> key_col = base.getKeyEntityColumn();
        std::shared_ptr<Criteria> criteria = base.createCriteria();
        criteria->andEqualTo(key_col->getProperty(), key);
        return deleteBy(base);
    }

    void use(const std::string& sql_conf) { m_sql_conf = sql_conf; }

private:
    std::string m_sql_conf;
};

}   // namespace pico

#endif   // !__PICO_MAPPER_COMMON_BASE_MAPPER_HPP__
