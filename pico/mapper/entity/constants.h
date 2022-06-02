#ifndef __PICO_MAPPER_ENTITY_CONSTANTS_H__
#define __PICO_MAPPER_ENTITY_CONSTANTS_H__

#include <string>

namespace pico {
namespace Constants {
const static std::string AND = "AND";
const static std::string OR = "OR";

const static std::string IS_NULL = "IS NULL";
const static std::string IS_NOT_NULL = "IS NOT NULL";

const static std::string EQUAL_TO = "=";
const static std::string NOT_EQUAL_TO = "<>";
const static std::string LESS_THAN = "<";
const static std::string LESS_THAN_OR_EQUAL_TO = "<=";
const static std::string GREATER_THAN = ">";
const static std::string GREATER_THAN_OR_EQUAL_TO = ">=";

const static std::string LIKE = "LIKE";
const static std::string NOT_LIKE = "NOT LIKE";

const static std::string IN = "IN";
const static std::string NOT_IN = "NOT IN";

const static std::string BETWEEN = "BETWEEN";
const static std::string NOT_BETWEEN = "NOT BETWEEN";

const static std::string REGEXP = "REGEXP";
const static std::string NOT_REGEXP = "NOT REGEXP";

const static std::string AS = "AS";
const static std::string DESC = "DESC";
const static std::string ASC = "ASC";

const static std::string ON = "ON";

const static std::string COUNT = "COUNT(1)";
const static std::string PLACEHOLDER = "?";
}   // namespace Constants
}   // namespace pico

#endif