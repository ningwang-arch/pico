#ifndef __PICO_MAPPER_ENTITY_BASE_HELPER_HPP__
#define __PICO_MAPPER_ENTITY_BASE_HELPER_HPP__

#include <memory>
#include <string>
#include <vector>

#include "../entity/constants.h"
#include "../entity/criteria.hpp"
#include "../entity/criterion.h"
#include "../entity/object.hpp"

namespace pico {
class BaseHelper
{
public:
    static std::string getConditionFromCriterion(const Criterion& criterion) {
        std::string cond = criterion.getCondition();
        if (criterion.isNoValue()) { cond += ""; }
        if (criterion.isSingleValue()) { cond += " " + Constants::PLACEHOLDER; }
        if (criterion.isBetweenValue()) {
            cond +=
                " " + Constants::PLACEHOLDER + " " + Constants::AND + " " + Constants::PLACEHOLDER;
        }
        if (criterion.isListValue()) {
            cond += " (";
            for (int i = 0; i < (int)criterion.getValues().size(); i++) {
                cond += i == 0 ? " " + Constants::PLACEHOLDER : " ," + Constants::PLACEHOLDER;
            }
            cond += " )";
        }
        return cond;
    }
    static std::string getConditionFromCriteria(const Criteria& criteria) {
        std::string answer;
        auto& cria = criteria.getCriteria();
        for (int i = 0; i < (int)cria.size(); ++i) {
            auto condition = getConditionFromCriterion(cria[i]);
            answer += 0 == i ? condition : " " + cria[i].getAndOr() + " " + condition;
        }
        return answer;
    }

    static std::vector<Object> getValuesFromOredCriteria(
        const std::vector<std::shared_ptr<Criteria>>& ored_criteria) {
        std::vector<Object> ret;
        for (auto& criteria : ored_criteria) {
            for (auto& criterion : criteria->getCriteria()) {
                ret.insert(ret.end(), criterion.getValues().begin(), criterion.getValues().end());
            }
        }
        return ret;
    }
};

}   // namespace pico

#endif
