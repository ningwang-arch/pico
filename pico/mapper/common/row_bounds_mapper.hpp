#ifndef __PICO_MAPPER_COMMON_RAW_BOUNDS_MAPPER_HPP__
#define __PICO_MAPPER_COMMON_RAW_BOUNDS_MAPPER_HPP__

#include <algorithm>
#include <memory>
#include <string>
#include <vector>


#include "criteria.hpp"
#include "entity/base.hpp"
#include "util.h"

namespace pico {
template<typename T>
class RowBoundsMapper
{
public:
    RowBoundsMapper() = default;
    RowBoundsMapper(const std::string& sql_conf)
        : m_sql_conf(std::move(sql_conf)) {}

    std::vector<T> selectByRowBounds(const T& record, const RowBounds& row_bounds) {
        Base<T> base;
        std::shared_ptr<Criteria> criteria = base.createCriteria();
        criteria->andEqualTo(record);
        base.limit(row_bounds.offset(), row_bounds.limit());
        return BaseMapper<T>().select(base, m_sql_conf);
    }

    std::vector<T> selectByRowBounds(const Base<T>& base, const RowBounds& row_bounds) {
        auto base_inner = base;
        std::shared_ptr<Criteria> criteria = base_inner.createCriteria();
        base_inner.limit(row_bounds.offset(), row_bounds.limit());
        return BaseMapper<T>().select(base_inner, m_sql_conf);
    }

    void use(const std::string& sql_conf) { m_sql_conf = sql_conf; }

private:
    std::string m_sql_conf;
};


}   // namespace  pico

#endif   // !__PICO_MAPPER_COMMON_RAW_BOUNDS_MAPPER_HPP__
