/*
 * This file is part of SQLogger.
 *
 * SQLogger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SQLogger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SQLogger. If not, see <https://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2025 Sergey K. sergey[no_spam]@greenblit.com
 */

#include "sqlogger/database/database_schema.h"

/**
 * @brief Construct a new TableBuilder object
 * @param tableName Name of the table to build
 */
DatabaseSchema::TableBuilder::TableBuilder(const std::string& tableName)
    : tableName(tableName) {}

DatabaseSchema::TableBuilder& DatabaseSchema::TableBuilder::addField(
    const std::string& name,
    FieldTypeResolver typeResolver,
    bool isPrimary,
    bool isNullable,
    bool isAutoincrement,
    bool isUnique,
    const std::string& defaultValue)
{
    fields.push_back({ name, typeResolver, isPrimary, isNullable, isAutoincrement, isUnique, defaultValue });
    return *this;
}

/**
 * @brief Add a field with standard type
 * @tparam T Type of the field (int32_t, int64_t, std::string, bool)
 * @param name Field name
 * @param isPrimary Whether the field is a primary key
 * @param isNullable Whether the field can be NULL
 * @param isAutoincrement Whether the field auto-increments
 * @param defaultValue Default value for the field
 * @return Reference to the TableBuilder for chaining
 */
template<typename T>
DatabaseSchema::TableBuilder& DatabaseSchema::TableBuilder::addStandardField(
    const std::string& name,
    bool isPrimary,
    bool isNullable,
    bool isAutoincrement,
    bool isUnique,
    const std::string& defaultValue)
{
    return addField(name, & resolveStandardType<T>, isPrimary, isNullable, isAutoincrement, isUnique, defaultValue);
}

/**
 * @brief Add a foreign key constraint
 * @param fieldName Name of the field that's a foreign key
 * @param referenceTable Table referenced by the foreign key
 * @param referenceField Field referenced by the foreign key
 * @return Reference to the TableBuilder for chaining
 * @throws std::logic_error if field hasn't been declared
 */
DatabaseSchema::TableBuilder& DatabaseSchema::TableBuilder::addForeignKey(
    const std::string& fieldName,
    const std::string& referenceTable,
    const std::string& referenceField)
{
    if(std::none_of(fields.begin(), fields.end(),
                    [ & ](const auto & f)
{
    return f.name == fieldName;
}))
    {
        throw std::logic_error("Field must be declared before adding foreign key");
    }

    foreignKeys.emplace(fieldName, std::make_pair(referenceTable, referenceField));
    return *this;
}

/**
 * @brief Build the table definition
 * @return BuiltTable structure containing the table definition
 */
DatabaseSchema::TableBuilder::BuiltTable DatabaseSchema::TableBuilder::build() const
{
    BuiltTable result{ tableName, fields, foreignKeys };
    return result;
}

/**
 * @brief Resolve standard type to database-specific type
 * @tparam T Type to resolve
 * @param dbType Database type
 * @return std::string Database-specific type string
 */
template<typename T>
std::string DatabaseSchema::TableBuilder::resolveStandardType(DataBaseType dbType)
{
    if constexpr(std::is_same_v<T, FieldType::Int32>)
    {
        switch(dbType)
        {
            case DataBaseType::SQLite:
                return DB_INT_TYPE_SQ;
            case DataBaseType::MySQL:
                return DB_INT_TYPE_MS;
            case DataBaseType::PostgreSQL:
                return DB_INT_TYPE_PG;
            default:
                return DB_INT_TYPE_DEF;
        }
    }
    else if constexpr(std::is_same_v<T, FieldType::Int64>)
    {
        switch(dbType)
        {
            case DataBaseType::SQLite:
                return DB_INT64_TYPE_SQ;
            case DataBaseType::MySQL:
                return DB_INT64_TYPE_MS;
            case DataBaseType::PostgreSQL:
                return DB_INT64_TYPE_PG;
            default:
                return DB_INT64_TYPE_DEF;
        }
    }
    else if constexpr(std::is_same_v<T, FieldType::String>)
    {
        switch(dbType)
        {
            case DataBaseType::SQLite:
                return DB_STRING_TYPE_SQ;
            case DataBaseType::MySQL:
                return DB_STRING_TYPE_MS;
            case DataBaseType::PostgreSQL:
                return DB_STRING_TYPE_PG;
            default:
                return DB_STRING_TYPE_DEF;
        }
    }
    else if constexpr(std::is_same_v<T, FieldType::Bool>)
    {
        switch(dbType)
        {
            case DataBaseType::SQLite:
                return DB_BOOL_TYPE_SQ;
            case DataBaseType::MySQL:
                return DB_BOOL_TYPE_MS;
            case DataBaseType::PostgreSQL:
                return DB_BOOL_TYPE_PG;
            default:
                return DB_BOOL_TYPE_DEF;
        }
    }
    else if constexpr(std::is_same_v<T, FieldType::DateTime>)
    {
        switch(dbType)
        {
            case DataBaseType::SQLite:
                return DB_DATETIME_TYPE_SQ;
            case DataBaseType::MySQL:
                return DB_DATETIME_TYPE_MS;
            case DataBaseType::PostgreSQL:
                return DB_DATETIMEL_TYPE_PG;
            default:
                return DB_DATETIME_TYPE_DEF;
        }
    }

    else if constexpr(std::is_same_v<T, FieldType::UUID>)
    {
        switch(dbType)
        {
            case DataBaseType::SQLite:
                return DB_UUID_TYPE_SQ;
            case DataBaseType::MySQL:
                return DB_UUID_TYPE_MS;
            case DataBaseType::PostgreSQL:
                return DB_UUID_TYPE_PG;
            default:
                return DB_UUID_TYPE_DEF;
        }
    }
}

/**
 * @brief Create a TableBuilder for a new table
 * @param tableName Name of the table to build
 * @return TableBuilder instance
 */
DatabaseSchema::TableBuilder DatabaseSchema::createTableBuilder(const std::string& tableName)
{
    return TableBuilder(tableName);
}

// Explicit template instantiations
template DatabaseSchema::TableBuilder& DatabaseSchema::TableBuilder::addStandardField<FieldType::Int32>(
    const std::string&, bool, bool, bool, bool, const std::string&);
template DatabaseSchema::TableBuilder& DatabaseSchema::TableBuilder::addStandardField<FieldType::Int64>(
    const std::string&, bool, bool, bool, bool, const std::string&);
template DatabaseSchema::TableBuilder& DatabaseSchema::TableBuilder::addStandardField<FieldType::String>(
    const std::string&, bool, bool, bool, bool, const std::string&);
template DatabaseSchema::TableBuilder& DatabaseSchema::TableBuilder::addStandardField<FieldType::Bool>(
    const std::string&, bool, bool, bool, bool, const std::string&);
template DatabaseSchema::TableBuilder& DatabaseSchema::TableBuilder::addStandardField<FieldType::DateTime>(
    const std::string&, bool, bool, bool, bool, const std::string&);
template DatabaseSchema::TableBuilder& DatabaseSchema::TableBuilder::addStandardField<FieldType::UUID>(
    const std::string&, bool, bool, bool, bool, const std::string&);