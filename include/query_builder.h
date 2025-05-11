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

#ifndef QUERY_BUILDER_H
#define QUERY_BUILDER_H

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <iostream>
#include "database_helper.h"
#include "log_entry.h"
#include "database_schema.h"
#include "sql_builder.h"

#ifdef USE_MONGODB
    #include "document_builder.h"
#endif

/**
 * @class QueryBuilder
 * @brief Utility class with static methods for building SQL queries.
 * @note This class cannot be instantiated. Use its static methods directly.
 */
class QueryBuilder
{
    public:

        /**
         * @brief Builds INSERT SQL statement
         * @param dbType Target database type
         * @param table Table name
         * @param values Vector of column-value pairs
         * @return Formatted INSERT statement
         */
        static std::string buildInsert(DataBaseType dbType,
                                       const std::string& table,
                                       const std::vector<std::pair<std::string, std::string>> & values);

        /**
         * @brief Builds SELECT SQL statement
         * @param dbType Target database type
         * @param table Table name
         * @param fields List of columns to select
         * @param filters WHERE clause conditions
         * @param orderBy ORDER BY clause
         * @param limit LIMIT value
         * @param offset OFFSET value
         * @return Formatted SELECT statement
         */
        static std::string buildSelect(DataBaseType dbType,
                                       const std::string& table,
                                       const std::vector<std::string> & fields,
                                       const std::vector<Filter> & filters,
                                       const std::string& orderBy = "",
                                       int limit = -1,
                                       int offset = -1);

        /**
         * @brief Builds UPDATE SQL statement
         * @param dbType Target database type
         * @param table Table name
         * @param setValues Values to update
         * @param filters WHERE clause conditions
         * @return Formatted UPDATE statement
         */
        static std::string buildUpdate(DataBaseType dbType,
                                       const std::string& table,
                                       const std::vector<std::pair<std::string, std::string>> & setValues,
                                       const std::vector<Filter> & filters);

        /**
         * @brief Builds DELETE SQL statement
         * @param dbType Target database type
         * @param table Table name
         * @param filters WHERE clause conditions
         * @return Formatted DELETE statement
         */
        static std::string buildDelete(DataBaseType dbType,
                                       const std::string& table,
                                       const std::vector<Filter> & filters);

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
         * @brief Builds CREATE INDEX statement
         * @param dbType Target database type
         * @param tableName Table name
         * @param indexName Index name
         * @param columns List of columns to index
         * @return Formatted CREATE INDEX statement
         */
        static std::string buildCreateIndex(
            DataBaseType dbType,
            const std::string& tableName,
            const std::string& indexName,
            const std::vector<std::string> & columns);

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

        QueryBuilder() = delete;
        QueryBuilder(const QueryBuilder&) = delete;
        QueryBuilder& operator=(const QueryBuilder&) = delete;
};

#endif // QUERY_BUILDER_H