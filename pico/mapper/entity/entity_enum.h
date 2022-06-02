#ifndef __PICO_MAPPER_ENTITY_ENTITY_ENUM_H__
#define __PICO_MAPPER_ENTITY_ENTITY_ENUM_H__

namespace pico {
/**
 * Column type.
 */
enum class ColumnType
{
    Null,   // common
    Id      // primary key
};

/**
 * primary key geneartion policy.
 */
enum class PrimaryKeyPolicy
{
    Null,   // common
    Id,     // auto increment
    UUID    // uuid
};

/**
 * Delete policy.
 */
enum class DeletePolicy
{
    Soft,   // soft delete
    Hard    // hard delete
};

/**
 * Convert policy.
 */
enum class Style
{
    Normal,   // origin value
    Camel,    // camel style
};

/**
 * Join policy.
 */
enum class JoinType
{
    Null,
    OneToOne,
    OneToMany,
    ManyToOne,
    ManyToMany
};

}   // namespace pico

#endif