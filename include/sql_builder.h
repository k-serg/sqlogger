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

#ifndef SQL_BUILDER_H
#define SQL_BUILDER_H

#include <string>
#include <vector>
#include "database_helper.h"
#include "database_schema.h"

/**
 * @brief Array of allowed foreign key actions
 */
constexpr char* ALLOWED_FK_ACTIONS[] = { "RESTRICT", "CASCADE", "SET NULL", "NO ACTION", "SET DEFAULT" };

/**
 * @class SQLBuilder
 * @brief Utility class with static methods for building SQL queries.
 * @note This class cannot be instantiated. Use its static methods directly.
 */
class SQLBuilder
{
    public:

        /**
         * @brief Builds a CREATE TABLE SQL query based on the table definition and database type
         *
         * @param table The table definition built using TableBuilder
         * @param dbType The target database type for which to generate the query
         * @return std::string The generated SQL CREATE TABLE query
         *
         * @details This function generates a database-specific CREATE TABLE statement with:
         * - Properly formatted identifiers based on database type
         * - Field type definitions including special handling for auto-increment fields
         * - Primary key constraints
         * - NOT NULL constraints where applicable
         * - Default values when specified
         * - Foreign key constraints
         *
         * @note For MongoDB and Mock databases, returns an empty string as they don't use SQL table creation
         * @note Handles PostgreSQL's SERIAL/BIGSERIAL types specially for auto-increment fields
         * @note Automatically adds "IF NOT EXISTS" clause to prevent errors on existing tables
         * @note Properly formats the query with newlines and indentation for readability
         */
        static std::string buildCreateTable(const DatabaseSchema::TableBuilder::BuiltTable& table,
                                            DataBaseType dbType);

        /**
         * @brief Builds query to check if table exists
         * @param dbType Target database type
         * @param table Table name to check
         * @return Formatted table existence query
         */
        static std::string buildTableExistsQuery(DataBaseType dbType, const std::string& table);

        /**
         * @brief Builds query to check if index exists
         * @param dbType Target database type
         * @param indexName Index name to check
         * @return Formatted index existence query
         */
        static std::string buildIndexExistsQuery(DataBaseType dbType, const std::string& indexName);

        /**
         * @brief Builds SQL INSERT statement
         * @param table Table name
         * @param values Column-value pairs
         * @param paramPrefix Prefix for parameters
         * @param dbType Database type
         * @return Formatted INSERT statement
         */
        static std::string buildSQLInsert(const std::string& table,
                                          const std::vector<std::pair<std::string, std::string>> & values,
                                          const std::string& paramPrefix,
                                          DataBaseType dbType);

        /**
         * @brief Builds SQL SELECT statement
         * @param fields Fields to select
         * @param table Table name
         * @param filters WHERE conditions
         * @param orderBy ORDER BY clause
         * @param limit LIMIT value
         * @param offset OFFSET value
         * @param paramPrefix Parameter prefix
         * @param dbType Database type
         * @return Formatted SELECT statement
         */
        static std::string buildSQLSelect(const std::vector<std::string> & fields,
                                          const std::string& table,
                                          const std::vector<Filter> & filters,
                                          const std::string& orderBy,
                                          int limit,
                                          int offset,
                                          const std::string& paramPrefix,
                                          DataBaseType dbType);

        /**
         * @brief Builds SQL UPDATE statement
         * @param table Table name
         * @param setValues Values to set
         * @param filters WHERE conditions
         * @param paramPrefix Parameter prefix
         * @return Formatted UPDATE statement
         */
        static std::string buildSQLUpdate(const std::string& table,
                                          const std::vector<std::pair<std::string, std::string>> & setValues,
                                          const std::vector<Filter> & filters,
                                          const std::string& paramPrefix);

        /**
         * @brief Builds SQL DELETE statement
         * @param table Table name
         * @param filters WHERE conditions
         * @param paramPrefix Parameter prefix
         * @return Formatted DELETE statement
         */
        static std::string buildSQLDelete(const std::string& table,
                                          const std::vector<Filter> & filters,
                                          const std::string& paramPrefix);

        /**
        * @brief Builds CREATE INDEX SQL statement
        * @param dbType Target database type
        * @param tableName Table name
        * @param indexName Index name
        * @param columns List of columns to index
        * @return Formatted CREATE INDEX statement
        */
        static std::string buildCreateIndexSQL(
            DataBaseType dbType,
            const std::string& tableName,
            const std::string& indexName,
            const std::vector<std::string> & columns);

        /**
        * @brief Escapes special characters in SQL values
        * @param dbType Target database type
        * @param value Value to escape
        * @return Escaped string
        */
        static std::string escapeValue(DataBaseType dbType, const std::string& value);

        /**
         * @brief Formats a value according to database type
         * @param dbType Target database type
         * @param value Value to format
         * @param type How to treat the value (Auto/String/Number)
         * @return Formatted value string
         */
        static std::string formatValue(DataBaseType dbType, const std::string& value, ValueType type = ValueType::Auto);

        /**
         * @brief Formats SQL identifiers (tables, columns)
         * @param dbType Target database type
         * @param identifier Identifier to format
         * @return Formatted identifier
         */
        static std::string formatIdentifier(DataBaseType dbType, const std::string& identifier);

        /**
        * @brief Builds WHERE clause from filters
        * @param dbType Database type
        * @param filters Filter conditions
        * @param paramPrefix Parameter prefix
        * @return Formatted WHERE clause
        */
        static std::string buildWhereClause(DataBaseType dbType,
                                            const std::vector<Filter> & filters,
                                            const std::string& paramPrefix);

        /**
        * @brief Resolves the appropriate auto-increment syntax for a given database type and field type
        *
        * @param dbType The database type for which to resolve the auto-increment syntax
        * @param fieldType The field type (used for PostgreSQL to determine SERIAL vs BIGSERIAL)
        * @return std::string The database-specific auto-increment keyword or type
        *
        * @details This function returns the correct auto-increment syntax based on:
        * - For SQLite: Returns "AUTOINCREMENT"
        * - For MySQL: Returns "AUTO_INCREMENT"
        * - For PostgreSQL: Returns "SERIAL" for integer fields or "BIGSERIAL" for bigint fields
        * - For other databases: Returns default auto-increment syntax
        *
        * @note PostgreSQL handles auto-increment differently by using special types (SERIAL/BIGSERIAL)
        * @note The fieldType parameter is only used for PostgreSQL to distinguish between SERIAL and BIGSERIAL
        * @throws Does not throw exceptions but returns a default value for unsupported database types
        */
        static std::string resolveAutoIncrement(DataBaseType dbType, const std::string& fieldType = "");

        /**
         * @brief Constructs a parameterized batch INSERT SQL statement optimized for the target database.
         * Generates a SQL query with proper parameter placeholders for bulk insertion of multiple rows.
         * The query format is adapted to the specific database dialect requirements.
         * @param table Name of the target table for insertion (will be properly quoted).
         * @param fields Vector of column names to insert (will be properly quoted).
         * @param numRows Number of rows to include in the batch operation.
         * @param dbType Database type determining SQL dialect and parameter style.
         * @return std::string The constructed parameterized INSERT query, or empty string for invalid input.
         * @note Returns empty string if fields vector is empty or numRows is zero
         * @note Uses database-specific parameter placeholders (?, $1, etc.)
         * @note Automatically applies proper identifier quoting based on database type
         */
        static std::string buildSQLBatchInsert(
            const std::string& table,
            const std::vector<std::string> & fields,
            const size_t numRows,
            const DataBaseType dbType);

        SQLBuilder() = delete;
        SQLBuilder(const SQLBuilder&) = delete;
        SQLBuilder& operator=(const SQLBuilder&) = delete;
};

#endif // !SQL_BUILDER_H
