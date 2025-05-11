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

#include <query_builder.h>

/**
 * @brief Builds an INSERT statement for the specified database type
 * @param dbType Target database type
 * @param table Name of the table to insert into
 * @param values std::vector of column-value std::pairs to insert
 * @return Formatted INSERT statement
 * @throws runtime_error If database type is unsupported
 */
std::string QueryBuilder::buildInsert(DataBaseType dbType,
                                      const std::string& table,
                                      const std::vector<std::pair<std::string, std::string>> & values)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
        case DataBaseType::SQLite:
        case DataBaseType::MySQL:
            return SQLBuilder::buildSQLInsert(table, values, "?", dbType);

        case DataBaseType::PostgreSQL:
            return SQLBuilder::buildSQLInsert(table, values, "$", dbType);

#ifdef USE_MONGODB
        case DataBaseType::MongoDB:
            return DocumentBuilder::buildMongoInsert(table, values);
#endif
        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
    }
}

/**
 * @brief Builds a SELECT SQL statement for the specified database type
 * @param dbType Target database type
 * @param table Name of the table to select from
 * @param fields List of columns to select
 * @param filters WHERE clause conditions
 * @param orderBy ORDER BY clause
 * @param limit LIMIT value
 * @param offset OFFSET value
 * @return Formatted SELECT statement
 * @throws runtime_error If database type is unsupported
 */
std::string QueryBuilder::buildSelect(DataBaseType dbType,
                                      const std::string& table,
                                      const std::vector<std::string> & fields,
                                      const std::vector<Filter> & filters,
                                      const std::string& orderBy,
                                      int limit,
                                      int offset)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
        case DataBaseType::SQLite:
        case DataBaseType::MySQL:
            return SQLBuilder::buildSQLSelect(fields, table, filters, orderBy, limit, offset, "?", dbType);

        case DataBaseType::PostgreSQL:
            return SQLBuilder::buildSQLSelect(fields, table, filters, orderBy, limit, offset, "$", dbType);

#ifdef USE_MONGODB
        case DataBaseType::MongoDB:
            return DocumentBuilder::buildMongoFind(fields, table, filters, orderBy, limit, offset);
#endif
        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
    }
}

/**
 * @brief Builds an UPDATE SQL statement for the specified database type
 * @param dbType Target database type
 * @param table Name of the table to update
 * @param setValues Values to update (column-value std::pairs)
 * @param filters WHERE clause conditions
 * @return Formatted UPDATE statement
 * @throws runtime_error If database type is unsupported
 */
std::string QueryBuilder::buildUpdate(DataBaseType dbType,
                                      const std::string& table,
                                      const std::vector<std::pair<std::string, std::string>> & setValues,
                                      const std::vector<Filter> & filters)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
        case DataBaseType::SQLite:
        case DataBaseType::MySQL:
            return SQLBuilder::buildSQLUpdate(table, setValues, filters, "?");

        case DataBaseType::PostgreSQL:
            return SQLBuilder::buildSQLUpdate(table, setValues, filters, "$");

#ifdef USE_MONGODB
        case DataBaseType::MongoDB:
            return DocumentBuilder::buildMongoUpdate(table, setValues, filters);
#endif
        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
    }
}

/**
 * @brief Builds a DELETE SQL statement for the specified database type
 * @param dbType Target database type
 * @param table Name of the table to delete from
 * @param filters WHERE clause conditions
 * @return Formatted DELETE statement
 * @throws runtime_error If database type is unsupported
 */
std::string QueryBuilder::buildDelete(DataBaseType dbType,
                                      const std::string& table,
                                      const std::vector<Filter> & filters)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
        case DataBaseType::SQLite:
        case DataBaseType::MySQL:
            return SQLBuilder::buildSQLDelete(table, filters, "?");

        case DataBaseType::PostgreSQL:
            return SQLBuilder::buildSQLDelete(table, filters, "$");

#ifdef USE_MONGODB
        case DataBaseType::MongoDB:
            return DocumentBuilder::buildMongoDelete(table, filters);
#endif
        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
    }
}

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
 * @note For MongoDB and Mock databases, returns an empty std::string as they don't use SQL table creation
 * @note Handles PostgreSQL's SERIAL/BIGSERIAL types specially for auto-increment fields
 * @note Automatically adds "IF NOT EXISTS" clause to prevent errors on existing tables
 * @note Properly formats the query with newlines and indentation for readability
 */
std::string QueryBuilder::buildCreateTable(const DatabaseSchema::TableBuilder::BuiltTable& table,
        DataBaseType dbType)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
            return "";

        case DataBaseType::SQLite:
        case DataBaseType::MySQL:
        case DataBaseType::PostgreSQL:
            return SQLBuilder::buildCreateTable(table, dbType);

        case DataBaseType::MongoDB:
            return ""; // MongoDB doesn't require table creation

        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
    }
}

/**
 * @brief Builds a CREATE INDEX statement
 * @param dbType Target database type
 * @param tableName Name of the table to index
 * @param indexName Name of the index to create
 * @param columns List of columns to include in the index
 * @return Formatted CREATE INDEX statement
 */
std::string QueryBuilder::buildCreateIndex(
    DataBaseType dbType,
    const std::string& tableName,
    const std::string& indexName,
    const std::vector<std::string> & columns)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
            return "";

        case DataBaseType::SQLite:
        case DataBaseType::MySQL:
        case DataBaseType::PostgreSQL:
            return SQLBuilder::buildCreateIndexSQL(dbType, tableName, indexName, columns);

        case DataBaseType::MongoDB:
#ifdef USE_MONGODB
            return DocumentBuilder::buildCreateIndexMongo(tableName, indexName, columns);
#endif;

        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
    }
}

/**
 * @brief Builds a query to check if a table exists
 * @param dbType Target database type
 * @param table Name of the table to check
 * @return Formatted table existence query
 * @throws runtime_error If database type is unsupported
 */
std::string QueryBuilder::buildTableExistsQuery(DataBaseType dbType, const std::string& table)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
            return "";

        case DataBaseType::PostgreSQL:
        case DataBaseType::MySQL:
        case DataBaseType::SQLite:
            return SQLBuilder::buildTableExistsQuery(dbType, table);

        case DataBaseType::MongoDB:
            return "";

        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
            break;
    }
}

/**
 * @brief Builds a query to check if an index exists
 * @param dbType Target database type
 * @param indexName Name of the index to check
 * @return Formatted index existence query
 * @throws runtime_error If database type is unsupported
 */
std::string QueryBuilder::buildIndexExistsQuery(DataBaseType dbType, const std::string& indexName)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
            return "";

        case DataBaseType::SQLite:
        case DataBaseType::MySQL:
        case DataBaseType::PostgreSQL:
            return SQLBuilder::buildIndexExistsQuery(dbType, indexName);

        case DataBaseType::MongoDB:
            return "";

        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
    }
};
