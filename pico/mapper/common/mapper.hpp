#ifndef __PICO_MAPPER_COMMON_MAPPER_HPP__
#define __PICO_MAPPER_COMMON_MAPPER_HPP__

#include "../entity/base.hpp"
#include "base_mapper.hpp"

namespace pico {
template<typename T>
class Mapper : public BaseMapper<T>
{};

}   // namespace pico

#endif   // ! __PICO_MAPPER_COMMON_MAPPER_HPP__
