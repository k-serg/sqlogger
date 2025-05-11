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

#ifndef DATABASE_SCHEMA_H
#define DATABASE_SCHEMA_H

#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <chrono>
#include "database_helper.h"

/// @brief String type definitions for SQLite
#define DB_STRING_TYPE_SQ "TEXT"
/// @brief String type definitions for MySQL
#define DB_STRING_TYPE_MS "VARCHAR(256)"
/// @brief String type definitions for PostgreSQL
#define DB_STRING_TYPE_PG "TEXT"
/// @brief Default string type definition
#define DB_STRING_TYPE_DEF "TEXT"

/// @brief Integer type definitions for SQLite
#define DB_INT_TYPE_SQ "INTEGER"
/// @brief Integer type definitions for MySQL
#define DB_INT_TYPE_MS "INTEGER"
/// @brief Integer type definitions for PostgreSQL
#define DB_INT_TYPE_PG "INTEGER"
/// @brief Default integer type definition
#define DB_INT_TYPE_DEF "INTEGER"

/// @brief 64-bit integer type definitions for SQLite
#define DB_INT64_TYPE_SQ "INTEGER"
/// @brief 64-bit integer type definitions for MySQL
#define DB_INT64_TYPE_MS "BIGINT"
/// @brief 64-bit integer type definitions for PostgreSQL
#define DB_INT64_TYPE_PG "BIGINT"
/// @brief Default 64-bit integer type definition
#define DB_INT64_TYPE_DEF "INTEGER"

/// @brief Boolean type definitions for SQLite
#define DB_BOOL_TYPE_SQ "INTEGER"
/// @brief Boolean type definitions for MySQL
#define DB_BOOL_TYPE_MS "BOOLEAN"
/// @brief Boolean type definitions for PostgreSQL
#define DB_BOOL_TYPE_PG "BOOLEAN"
/// @brief Default boolean type definition
#define DB_BOOL_TYPE_DEF "INTEGER"

/// @brief Datetime type definitions for SQLite
#define DB_DATETIME_TYPE_SQ "DATETIME"
/// @brief Datetime type definitions for MySQL
#define DB_DATETIME_TYPE_MS "DATETIME"
/// @brief Datetime type definitions for PostgreSQL
#define DB_DATETIMEL_TYPE_PG "TIMESTAMP"
/// @brief Default Datetime type definition
#define DB_DATETIME_TYPE_DEF "TEXT"

/// @brief UUID type definitions for SQLite
#define DB_UUID_TYPE_SQ "TEXT"
/// @brief UUID type definitions for MySQL
#define DB_UUID_TYPE_MS "CHAR(36)"
/// @brief UUID type definitions for PostgreSQL
#define DB_UUID_TYPE_PG "UUID"
/// @brief Default UUID type definition
#define DB_UUID_TYPE_DEF "TEXT"

/// @brief Autoincrement definition for SQLite
#define DB_AUTOINCREMENT_SQ "AUTOINCREMENT"
/// @brief Autoincrement definition for MySQL
#define DB_AUTOINCREMENT_MS "AUTO_INCREMENT"
/// @brief Autoincrement definition for PostgreSQL
#define DB_AUTOINCREMENT_PG "SERIAL"
/// @brief Big autoincrement definition for PostgreSQL
#define DB_AUTOINCREMENT_BIG_PG "BIGSERIAL"
/// @brief Default autoincrement definition
#define DB_AUTOINCREMENT_DEF "AUTOINCREMENT"

namespace FieldType
{
    /// @brief Alias for bool
    using Bool = bool;

    /// @brief Alias for int32_t
    using Int32 = int32_t;

    /// @brief Alias for int64_t
    using Int64 = int64_t;

    /// @brief Alias for std::string
    using String = std::string;

    /// @brief Alias for std::chrono::system_clock::time_point
    using DateTime = std::chrono::system_clock::time_point;

    /// @brief Alias for std::string
    using UUID = std::string;
};

/**
 * @class DatabaseSchema
 * @brief Provides functionality to define database schema and table structures
 */
class DatabaseSchema
{
    public:

        /// @brief Function type for resolving field types based on database type
        using FieldTypeResolver = std::function<std::string(DataBaseType)>;

        /**
         * @class TableBuilder
         * @brief Builder class for constructing table definitions
         */
        class TableBuilder
        {
            public:
                /**
                 * @brief Construct a new TableBuilder object
                 * @param tableName Name of the table to build
                 */
                explicit TableBuilder(const std::string& tableName);

                /**
                 * @brief Add a field with custom type resolver
                 * @param name Field name
                 * @param typeResolver Function to resolve field type based on database
                 * @param isPrimary Whether the field is a primary key
                 * @param isNullable Whether the field can be NULL
                 * @param isAutoincrement Whether the field auto-increments
                 * @param defaultValue Default value for the field
                 * @return Reference to the TableBuilder for chaining
                 */
                TableBuilder& addField(
                    const std::string& name,
                    FieldTypeResolver typeResolver,
                    bool isPrimary = false,
                    bool isNullable = true,
                    bool isAutoincrement = false,
                    bool isUnique = false,
                    const std::string& defaultValue = "");

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
                TableBuilder& addStandardField(
                    const std::string& name,
                    bool isPrimary = false,
                    bool isNullable = true,
                    bool isAutoincrement = false,
                    bool isUnique = false,
                    const std::string& defaultValue = "");

                /**
                 * @brief Add a foreign key constraint
                 * @param fieldName Name of the field that's a foreign key
                 * @param referenceTable Table referenced by the foreign key
                 * @param referenceField Field referenced by the foreign key
                 * @return Reference to the TableBuilder for chaining
                 * @throws std::logic_error if field hasn't been declared
                 */
                TableBuilder& addForeignKey(
                    const std::string& fieldName,
                    const std::string& referenceTable,
                    const std::string& referenceField);

                /**
                 * @brief Structure representing a built table definition
                 */
                struct BuiltTable
                {
                    std::string name; ///< Name of the table
                    /**
                     * @brief Structure representing a table field
                     */
                    struct Field
                    {
                        std::string name;               ///< Field name
                        FieldTypeResolver getDbType;    ///< Type resolver function
                        bool isPrimary;                 ///< Is primary key
                        bool isNullable;                ///< Is nullable
                        bool isAutoincrement;           ///< Is auto-incrementing
                        bool isUnique;                  ///< Is unique
                        std::string defaultValue;       ///< Default value
                    };
                    std::vector<Field> fields; ///< List of fields in the table
                    std::unordered_map<std::string, std::pair<std::string, std::string>> foreignKeys; ///< Foreign key constraints
                };

                /**
                 * @brief Build the table definition
                 * @return BuiltTable structure containing the table definition
                 */
                BuiltTable build() const;

            private:
                std::string tableName; ///< Name of the table being built
                std::vector<BuiltTable::Field> fields; ///< Fields in the table
                std::unordered_map<std::string, std::pair<std::string, std::string>> foreignKeys; ///< Foreign key constraints

                /**
                 * @brief Resolve standard type to database-specific type
                 * @tparam T Type to resolve
                 * @param dbType Database type
                 * @return std::string Database-specific type string
                 */
                template<typename T>
                static std::string resolveStandardType(DataBaseType dbType);
        };

        /**
         * @brief Create a TableBuilder for a new table
         * @param tableName Name of the table to build
         * @return TableBuilder instance
         */
        static TableBuilder createTableBuilder(const std::string& tableName);

};

#endif // DATABASE_SCHEMA_H