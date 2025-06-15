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

#include "sqlogger/database/sql_builder.h"

/**
 * @brief Builds a standard SQL INSERT statement
 * @param table Name of the table to insert into
 * @param values std::vector of column-value std::pairs to insert
 * @param paramPrefix Prefix for parameter placeholders
 * @param dbType Target database type
 * @return Formatted SQL INSERT statement
 * @throws runtime_error If database type is unsupported
 */
std::string SQLBuilder::buildSQLInsert(const std::string& table,
                                       const std::vector<std::pair<std::string, std::string>> & values,
                                       const std::string& paramPrefix,
                                       DataBaseType dbType)
{
    std::stringstream query;
    query << "INSERT INTO " << formatIdentifier(dbType, table) << " (";

    // Fields
    for(size_t i = 0; i < values.size(); ++i)
    {
        if(i > 0) query << ", ";
        query << formatIdentifier(dbType, values[i].first);
    }

    // Values
    query << ") VALUES (";
    for(size_t i = 0; i < values.size(); ++i)
    {
        if(i > 0) query << ", ";

        // Generate placeholder based on database type
        switch(dbType)
        {
            case DataBaseType::PostgreSQL:
                query << paramPrefix << (i + 1);  // $1, $2, $3
                break;
            case DataBaseType::Mock:
            case DataBaseType::MySQL:
            case DataBaseType::SQLite:
                query << "?";  // ?
                break;
            default:
                throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
        }
    }
    query << ")";

    return query.str();
}

/**
 * @brief Builds a standard SQL SELECT statement
 * @param fields List of columns to select
 * @param table Name of the table to select from
 * @param filters WHERE clause conditions
 * @param orderBy ORDER BY clause
 * @param limit LIMIT value
 * @param offset OFFSET value
 * @param paramPrefix Prefix for parameter placeholders
 * @param dbType Target database type
 * @return Formatted SQL SELECT statement
 * @throws exception If query building fails
 */
std::string SQLBuilder::buildSQLSelect(
    const std::vector<std::string> & fields,
    const std::string& table,
    const std::vector<Filter> & filters,
    const std::string& orderBy,
    int limit,
    int offset,
    const std::string& paramPrefix,
    DataBaseType dbType)
{
    std::stringstream query;
    query << "SELECT ";

    // Fields
    if(fields.empty())
    {
        query << "*";
    }
    else
    {
        for(size_t i = 0; i < fields.size(); ++i)
        {
            if(i > 0) query << ", ";
            query << formatIdentifier(dbType, fields[i]);
        }
    }

    // Table
    query << " FROM " << formatIdentifier(dbType, table);

    // WHERE
    if(!filters.empty())
    {
        query << " WHERE " << buildWhereClause(dbType, filters, paramPrefix);
    }

    // ORDER BY
    if(!orderBy.empty())
    {
        query << " ORDER BY " << formatIdentifier(dbType, orderBy);
    }

    // LIMIT/OFFSET
    if(limit > 0)
    {
        query << " LIMIT " << limit;

        if(offset > 0)
        {
            if(dbType == DataBaseType::MySQL)
            {
                query << " OFFSET " << offset;
            }
            else
            {
                query << " OFFSET " << offset;
            }
        }
    }

    return query.str();
}

/**
 * @brief Builds a standard SQL UPDATE statement
 * @param table Name of the table to update
 * @param setValues Values to update (column-value std::pairs)
 * @param filters WHERE clause conditions
 * @param paramPrefix Prefix for parameter placeholders
 * @return Formatted SQL UPDATE statement
 */
std::string SQLBuilder::buildSQLUpdate(const std::string& table,
                                       const std::vector<std::pair<std::string, std::string>> & setValues,
                                       const std::vector<Filter> & filters,
                                       const std::string& paramPrefix)
{
    std::stringstream query;
    query << "UPDATE " << table << " SET ";

    // SET clause
    for(size_t i = 0; i < setValues.size(); ++i)
    {
        if(i > 0) query << ", ";
        query << setValues[i].first << " = " << paramPrefix << i + 1;
    }

    // WHERE clause
    if(!filters.empty())
    {
        query << " WHERE " << buildWhereClause(DataBaseType::PostgreSQL, filters,
                                               paramPrefix + std::to_string(setValues.size() + 1));
    }

    return query.str();
}

/**
 * @brief Builds a standard SQL DELETE statement
 * @param table Name of the table to delete from
 * @param filters WHERE clause conditions
 * @param paramPrefix Prefix for parameter placeholders
 * @return Formatted SQL DELETE statement
 */
std::string SQLBuilder::buildSQLDelete(const std::string& table,
                                       const std::vector<Filter> & filters,
                                       const std::string& paramPrefix)
{
    std::stringstream query;
    query << "DELETE FROM " << table;

    // WHERE clause
    if(!filters.empty())
    {
        query << " WHERE " << buildWhereClause(DataBaseType::PostgreSQL, filters, paramPrefix + "1");
    }

    return query.str();
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
std::string SQLBuilder::buildCreateTable(const DatabaseSchema::TableBuilder::BuiltTable& table,
        DataBaseType dbType)
{
    std::string query = "CREATE TABLE IF NOT EXISTS " + formatIdentifier(dbType, table.name) + " (\n";

    for(const auto & field : table.fields)
    {
        query += "  " + formatIdentifier(dbType, field.name) + " ";


        // PG && AUTOINCREMENT (BIG)SERIAL
        if(field.isAutoincrement && dbType == DataBaseType::PostgreSQL)
        {
            query += resolveAutoIncrement(dbType, field.getDbType(dbType));
        }

        // !AUTOINCREMENT INTEGER || !PG
        if(!field.isAutoincrement || dbType != DataBaseType::PostgreSQL)
        {
            query += field.getDbType(dbType);
        }

        // PRIMARY
        if(field.isPrimary)
        {
            query += " PRIMARY KEY";

            // !PG AUTOINCREMENT
            if(field.isAutoincrement && dbType != DataBaseType::PostgreSQL)
            {
                query += " " + resolveAutoIncrement(dbType);
            }
        }

        // UNIQUE
        if(field.isUnique) query += " UNIQUE";

        // NOT NULL
        if(!field.isNullable) query += " NOT NULL";

        // DEFAULT
        if(!field.defaultValue.empty()) query += " DEFAULT " + field.defaultValue;
        query += ",\n";
    }

    // FOREIGN KEY
    for(const auto & [field, ref] : table.foreignKeys)
    {
        query += "  FOREIGN KEY (" + formatIdentifier(dbType, field) + ") REFERENCES " +
                 formatIdentifier(dbType, ref.first) + " (" + formatIdentifier(dbType, ref.second) + "),\n";
    }

    if(!table.fields.empty() || !table.foreignKeys.empty())
    {
        query = query.substr(0, query.size() - 2);
    }
    query += "\n);";

    return query;
}

/**
* @brief Builds CREATE INDEX SQL statement
* @param dbType Target database type
* @param tableName Table name
* @param indexName Index name
* @param columns List of columns to index
* @return Formatted CREATE INDEX statement
*/
std::string SQLBuilder::buildCreateIndexSQL(
    DataBaseType dbType,
    const std::string& tableName,
    const std::string& indexName,
    const std::vector<std::string> & columns)
{
    if(columns.empty())
    {
        return "";
    }

    switch(dbType)
    {
        case DataBaseType::SQLite:
        case DataBaseType::PostgreSQL:
        {
            std::stringstream query;
            query << "CREATE INDEX IF NOT EXISTS " << indexName
                  << " ON " << tableName << " (";
            for(size_t i = 0; i < columns.size(); ++i)
            {
                if(i > 0) query << ", ";
                query << columns[i];
            }
            query << ")";
            return query.str();
        }

        case DataBaseType::MySQL:
        {
            std::stringstream query;
            query << "ALTER TABLE " << tableName << " ADD INDEX "
                  << indexName << " (";
            for(size_t i = 0; i < columns.size(); ++i)
            {
                if(i > 0) query << ", ";
                query << columns[i];
            }
            query << ")";
            return query.str();
        }

        default:
            return "";
    }
}

/**
 * @brief Builds a query to check if a table exists
 * @param dbType Target database type
 * @param table Name of the table to check
 * @return Formatted table existence query
 * @throws runtime_error If database type is unsupported
 */
std::string SQLBuilder::buildTableExistsQuery(DataBaseType dbType, const std::string& table)
{
    switch(dbType)
    {
        case DataBaseType::SQLite:
            return "SELECT 1 FROM sqlite_master WHERE name = " + formatValue(dbType, table);

        case DataBaseType::MySQL:
            return "SELECT 1 FROM information_schema.tables WHERE table_name = " + formatValue(dbType, table);

        case DataBaseType::PostgreSQL:
            return "SELECT 1 FROM pg_tables WHERE tablename = " + formatValue(dbType, table);

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
std::string SQLBuilder::buildIndexExistsQuery(DataBaseType dbType, const std::string& indexName)
{
    switch(dbType)
    {
        case DataBaseType::SQLite:
            return "SELECT 1 FROM sqlite_master "
                   "WHERE type = 'index' AND name = " + formatValue(dbType, indexName);

        case DataBaseType::MySQL:
            return "SELECT 1 FROM information_schema.statistics "
                   "WHERE index_name = " + formatValue(dbType, indexName);

        case DataBaseType::PostgreSQL:
            return "SELECT 1 FROM pg_indexes WHERE indexname = " + formatValue(dbType, indexName);

        default:
            throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
    }
};

/**
 * @brief Builds a WHERE clause from filters
 * @param dbType Target database type
 * @param filters Filter conditions
 * @param paramPrefix Prefix for parameter placeholders
 * @return Formatted WHERE clause
 * @throws runtime_error If database type is unsupported
 */
std::string SQLBuilder::buildWhereClause(DataBaseType dbType,
        const std::vector<Filter> & filters,
        const std::string& paramPrefix)
{
    std::stringstream whereClause;

    for(size_t i = 0; i < filters.size(); ++i)
    {
        if(i > 0) whereClause << " AND ";

        // Format condition based on database type
        switch(dbType)
        {
            case DataBaseType::Mock:
            case DataBaseType::SQLite:
            case DataBaseType::MySQL:
                whereClause << formatIdentifier(dbType, filters[i].field) << " "
                            << filters[i].op << " ";
                if(!paramPrefix.empty())
                {
                    whereClause << paramPrefix;
                }
                else
                {
                    whereClause << formatValue(dbType, filters[i].value, ValueType::Auto);
                }
                break;

            case DataBaseType::PostgreSQL:
                whereClause << formatIdentifier(dbType, filters[i].field) << " "
                            << filters[i].op << " "
                            << paramPrefix << std::to_string(i + 1);
                break;

            default:
                throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
        }
    }
    return whereClause.str();
}

/**
 * @brief Escapes special characters in SQL values
 * @param dbType Target database type
 * @param value Value to escape
 * @return Escaped std::string
 */
std::string SQLBuilder::escapeValue(DataBaseType dbType, const std::string& value)
{
    if(value.empty()) return value;

    std::string escaped;
    escaped.reserve(value.size() * 2); // Pre-allocate memory

    switch(dbType)
    {
        case DataBaseType::Mock:
            return value;

        case DataBaseType::SQLite:
            // SQLite - double single quotes
            for(char c : value)
            {
                if(c == '\'') escaped += '\'';
                escaped += c;
            }
            break;

        case DataBaseType::MySQL:
            // MySQL requires escaping: \ ', \ ", \ \, \n, \r, \0, \Z
            for(char c : value)
            {
                switch(c)
                {
                    case '\'':
                        escaped += "\\'";
                        break;
                    case '"':
                        escaped += "\\\"";
                        break;
                    case '\\':
                        escaped += "\\\\";
                        break;
                    case '\n':
                        escaped += "\\n";
                        break;
                    case '\r':
                        escaped += "\\r";
                        break;
                    case 0:
                        escaped += "\\0";
                        break;
                    case 0x1A:
                        escaped += "\\Z";
                        break; // Ctrl+Z
                    default:
                        escaped += c;
                        break;
                }
            }
            break;

        case DataBaseType::PostgreSQL:
            // PostgreSQL requires escaping only "'" and "\"
            for(char c : value)
            {
                if(c == '\'' || c == '\\')
                {
                    escaped += '\\';
                }
                escaped += c;
            }
            break;

        default:
            return value;
    }

    return escaped;
}

/**
 * @brief Formats a value according to database type
 * @param dbType Target database type
 * @param value Value to format
 * @param type How to treat the value (Auto/std::string/Number)
 * @return Formatted value std::string
 */
std::string SQLBuilder::formatValue(DataBaseType dbType, const std::string& value, ValueType type)
{
    // Handle special cases
    if(value == "NULL" || value == "CURRENT_TIMESTAMP")
    {
        return value;
    }

    // Determine value type
    bool treatAsString = true;
    if(type == ValueType::Number)
    {
        treatAsString = false;
    }
    else if(type == ValueType::Auto)
    {
        treatAsString = !LogHelper::isNumeric(value);
    }

    // Format value
    if(treatAsString)
    {
        std::string escaped = escapeValue(dbType, value);
        switch(dbType)
        {
            case DataBaseType::MySQL:
                return "\"" + escaped + "\"";
            case DataBaseType::PostgreSQL:
                return "'" + escaped + "'";
            case DataBaseType::SQLite:
                return "'" + escaped + "'";
            default:
                return escaped;
        }
    }

    return value; // Numeric constants don't need escaping
}

/**
 * @brief Formats SQL identifiers (tables, columns)
 * @param dbType Target database type
 * @param identifier Identifier to format
 * @return Formatted identifier
 */
std::string SQLBuilder::formatIdentifier(DataBaseType dbType, const std::string& identifier)
{
    if(identifier.empty()) return identifier;

    char quote_char;
    switch(dbType)
    {
        case DataBaseType::MySQL:
            quote_char = '`';
            break;
        case DataBaseType::PostgreSQL:
            quote_char = '"';
            break;
        case DataBaseType::SQLite:
            quote_char = '"';
            break;
        default:
            return identifier;
    }

    std::string escaped;
    escaped.reserve(identifier.size() + 2);
    escaped += quote_char;

    for(char c : identifier)
    {
        if(c == quote_char) escaped += quote_char;     // Double quotes
        escaped += c;
    }

    escaped += quote_char;
    return escaped;
}

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
std::string SQLBuilder::resolveAutoIncrement(DataBaseType dbType, const std::string& fieldType)
{
    switch(dbType)
    {
        case DataBaseType::SQLite:
            return DB_AUTOINCREMENT_SQ;
            break;

        case DataBaseType::MySQL:
            return DB_AUTOINCREMENT_MS;
            break;

        case DataBaseType::PostgreSQL:
        {
            if(fieldType == DB_INT_TYPE_PG) return DB_AUTOINCREMENT_PG;
            else if(fieldType == DB_INT64_TYPE_PG) return DB_AUTOINCREMENT_BIG_PG;
            return DB_AUTOINCREMENT_PG;
        }
        break;

        default:
            return DB_AUTOINCREMENT_DEF;
    }
};

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
std::string SQLBuilder::buildSQLBatchInsert(
    const std::string& table,
    const std::vector<std::string> & fields,
    const size_t numRows,
    const DataBaseType dbType)
{
    const std::string paramPrefix = DataBaseHelper::databaseTypePrefix(dbType);

    if(fields.empty() || numRows == 0)
    {
        return "";
    }

    std::stringstream query;

    // INSERT clause
    query << "INSERT INTO " << formatIdentifier(dbType, table) << " (";

    // Field names
    for(size_t i = 0; i < fields.size(); ++i)
    {
        if(i > 0) query << ", ";
        query << formatIdentifier(dbType, fields[i]);
    }
    query << ") VALUES ";

    // Value placeholders
    for(size_t row = 0; row < numRows; ++row)
    {
        if(row > 0) query << ", ";
        query << "(";

        for(size_t col = 0; col < fields.size(); ++col)
        {
            if(col > 0) query << ", ";

            switch(dbType)
            {
                case DataBaseType::PostgreSQL:
                    query << paramPrefix << (row* fields.size() + col + 1);
                    break;
                default:
                    query << paramPrefix;
            }
        }
        query << ")";
    }

    return query.str();
}
