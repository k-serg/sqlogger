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

#include "database_helper.h"

bool DataBaseHelper::isDataBaseEmbedded(const DataBaseType& dbType)
{
    return !isDataBaseServer(dbType);
}

bool DataBaseHelper::isDataBaseServer(const DataBaseType& dbType)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
        case DataBaseType::SQLite:
            return false;
            break;

        case DataBaseType::MySQL:
        case DataBaseType::PostgreSQL:
        case DataBaseType::MongoDB:
            return true;
            break;

        default:
            throw std::invalid_argument(ERR_MSG_UNSUPPORTED_DB);
            break;
    }
}

/**
 * @brief Checks if a database type is supported in the current build configuration.
 * Determines whether the database type is available based on compile-time
 * feature flags (USE_MYSQL, USE_POSTGRESQL, etc.). Mock and SQLite are always
 * supported.
 * @param dbType The database type to check
 * @return bool True if the database type is supported, false otherwise
 * @see DataBaseType
 */
bool DataBaseHelper::isDataBaseSupported(const DataBaseType& dbType)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
        case DataBaseType::SQLite:
            return true;
            break;

        case DataBaseType::MySQL:
        {
#ifdef USE_MYSQL
            return true;
#else
            return false;
#endif
        }
        break;

        case DataBaseType::PostgreSQL:
        {
#ifdef USE_POSTGRESQL
            return true;
#else
            return false;
#endif
        }
        break;

        case DataBaseType::MongoDB:
        {
#ifdef USE_MONGODB
            return true;
#else
            return false;
#endif
        }
        break;

        default:
            return false;
            break;
    }
}

/**
 * @brief Gets the default network port number for a specified database type.
 * Returns the standard/default port number used by the database server.
 * For database types that don't use network ports (like SQLite), returns
 * DB_DEFAULT_PORT_NOT_SUPPORTED.
 * @param dbType The database type to query
 * @return int Default port number or DB_DEFAULT_PORT_NOT_SUPPORTED if not applicable
 * @see DataBaseType
 */
int DataBaseHelper::getDataBaseDefaultPort(const DataBaseType& dbType)
{
    switch(dbType)
    {
        case DataBaseType::Mock:
            return DB_DEFAULT_PORT_NOT_SUPPORTED;
            break;

        case DataBaseType::SQLite:
            return DB_DEFAULT_PORT_NOT_SUPPORTED;
            break;

        case DataBaseType::MySQL:
            return DB_DEFAULT_PORT_MYSQL;
            break;

        case DataBaseType::PostgreSQL:
            return DB_DEFAULT_PORT_POSTGRESQL;
            break;

        case DataBaseType::MongoDB:
            return DB_DEFAULT_PORT_MONGODB;
            break;

        default:
            return DB_DEFAULT_PORT_NOT_SUPPORTED;
            break;
    }
}

/**
 * @brief Converts a string representation of a database type to the corresponding enum value.
 * @param stringType The string representation of the database type.
 * @return The corresponding DataBaseType enum value.
 */
DataBaseType DataBaseHelper::stringToDatabaseType(const std::string& stringType)
{
    if(LogHelper::toUpperCase(stringType) == LogHelper::toUpperCase(DB_TYPE_STR_MOCK)) return DataBaseType::Mock;
    if(LogHelper::toUpperCase(stringType) == LogHelper::toUpperCase(DB_TYPE_STR_SQLITE)) return DataBaseType::SQLite;
    if(LogHelper::toUpperCase(stringType) == LogHelper::toUpperCase(DB_TYPE_STR_MYSQL)) return DataBaseType::MySQL;
    if(LogHelper::toUpperCase(stringType) == LogHelper::toUpperCase(DB_TYPE_STR_POSTGRESQL)) return DataBaseType::PostgreSQL;
    if(LogHelper::toUpperCase(stringType) == LogHelper::toUpperCase(DB_TYPE_STR_MONGODB)) return DataBaseType::MongoDB;
    throw std::invalid_argument(ERR_MSG_UNSUPPORTED_DB);
};

/**
 * @brief Converts a DataBaseType enum value to its string representation.
 * @param type The DataBaseType enum value.
 * @return The string representation of the database type.
 */
std::string DataBaseHelper::databaseTypeToString(const DataBaseType& type)
{
    switch(type)
    {
        case DataBaseType::Mock:
            return DB_TYPE_STR_MOCK;
            break;
        case DataBaseType::SQLite:
            return DB_TYPE_STR_SQLITE;
            break;
        case DataBaseType::MySQL:
            return DB_TYPE_STR_MYSQL;
            break;
        case DataBaseType::PostgreSQL:
            return DB_TYPE_STR_POSTGRESQL;
            break;
        case DataBaseType::MongoDB:
            return DB_TYPE_STR_MONGODB;
            break;
        default:
            throw std::invalid_argument(ERR_MSG_UNSUPPORTED_DB);
            break;
    }
};

/**
 * @brief Gets the database-specific parameter prefix string for the given database type.
 * @param type The database type to get the prefix for.
 * @return std::string The parameter prefix string specific to the database type.
 * @throw std::invalid_argument If an unsupported database type is provided.
 * @note For MongoDB, returns an empty string as it doesn't use parameter prefixes.
 * @see DB_PARAM_PREFIX_MOCK, DB_PARAM_PREFIX_SQLITE, DB_PARAM_PREFIX_MYSQL, DB_PARAM_PREFIX_POSTGRESQL.
 */
std::string DataBaseHelper::databaseTypePrefix(const DataBaseType& type)
{
    switch(type)
    {
        case DataBaseType::Mock:
            return DB_PARAM_PREFIX_MOCK;
            break;
        case DataBaseType::SQLite:
            return DB_PARAM_PREFIX_SQLITE;
            break;
        case DataBaseType::MySQL:
            return DB_PARAM_PREFIX_MYSQL;
            break;
        case DataBaseType::PostgreSQL:
            return DB_PARAM_PREFIX_POSTGRESQL;
            break;
        case DataBaseType::MongoDB:
            return "";
            break;
        default:
            throw std::invalid_argument(ERR_MSG_UNSUPPORTED_DB);
            break;
    }
};

/**
 * @brief Gets the maximum recommended batch size for the given database type.
 * @param type The database type to get the batch size limit for.
 * @return int Maximum recommended batch size for the database type.
 * @throw std::invalid_argument If an unsupported database type is provided.
 * @note Returns DB_BATCH_NOT_SUPPORTED (-1) for Mock and MongoDB databases (no batching supported).
 * @see DB_MAX_BATCH_SQLITE, DB_MAX_BATCH_MYSQL, DB_MAX_BATCH_POSTGRESQL.
 */
int DataBaseHelper::getMaxBatchSize(const DataBaseType& type)
{
    switch(type)
    {
        case DataBaseType::Mock:
            return DB_BATCH_NOT_SUPPORTED;
            break;
        case DataBaseType::SQLite:
            return DB_MAX_BATCH_SQLITE;
            break;
        case DataBaseType::MySQL:
            return DB_MAX_BATCH_MYSQL;
            break;
        case DataBaseType::PostgreSQL:
            return DB_MAX_BATCH_POSTGRESQL;
            break;
        case DataBaseType::MongoDB:
            return DB_BATCH_NOT_SUPPORTED;
            break;
        default:
            throw std::invalid_argument(ERR_MSG_UNSUPPORTED_DB);
            break;
    }
}

/**
 * @brief Escapes backslashes in a string.
 * @param input The input string to escape.
 * @return The string with escaped backslashes.
 */
std::string DataBaseHelper::escapeBackslashes(const std::string& input)
{
    std::string result;
    for(size_t i = 0; i < input.size(); ++i)
    {
        if(input[i] == '\\')
        {
            if(i + 1 < input.size() && input[i + 1] == '\\')
            {
                result += '\\';
                ++i;
            }
            else
            {
                result += '\\';
            }
        }
        result += input[i];
    }
    return result;
};

/**
* @brief Checks if a connection string follows URI format.
*
* @param connectionString The connection string to validate
* @return true if the string matches standard URI format (scheme://[authority][path][?query]),
*         false otherwise
*
* @details Validates the basic URI structure:
*          - Must contain "://" separator
*          - Scheme (before ://) must consist of alphanumeric chars + '+'/'.'/'-'
*          - Minimal valid example: "scheme://"
*
* @note Does not validate the URI components beyond basic format checking.
* @note Returns false for empty strings or strings without ://
*
* @example isUriFormat("mysql://user@host:3306/db") → true
* @example isUriFormat("host=localhost;port=3306") → false
* @example isUriFormat("sqlite:/path/to/file.db") → false (missing //)
*/
bool DataBaseHelper::isUriFormat(const std::string& connectionString)
{
    if(connectionString.empty())
    {
        return false;
    }

    // Find scheme separator
    const size_t scheme_end = connectionString.find("://");
    if(scheme_end == std::string::npos)
    {
        return false;
    }

    // Validate scheme (RFC 3986)
    const std::string scheme = connectionString.substr(0, scheme_end);
    return std::all_of(scheme.begin(), scheme.end(), [](char c)
    {
        return std::isalnum(c) || c == '+' || c == '.' || c == '-';
    });
};

/**
* @brief Parses a key-value pair from a delimited string.
*
* @param str The input string containing key-value pairs (e.g., "host=localhost;port=3306")
* @param key The key to search for in the string (e.g., "host")
* @param delimiter The character that separates key-value pairs (e.g., ';' or ' ')
* @return std::optional<std::string> The value associated with the key if found, std::nullopt otherwise
*
* @details The method searches through the string for segments separated by the delimiter,
*          then looks for the specified key followed by an equals sign ('='). If found,
*          it returns the portion after the equals sign.
*
* @note The comparison between keys is exact (case-sensitive).
* @note If multiple segments contain the same key, only the first occurrence is returned.
* @note The delimiter character cannot appear in the key or value portions.
* @note Empty values (e.g., "key=") will return an empty string (not std::nullopt).
*
* @example parseKeyValueString("host=localhost;port=3306", "port", ';') returns "3306"
* @example parseKeyValueString("name=John age=30", "age", ' ') returns "30"
*/
std::optional<std::string> DataBaseHelper::parseKeyValueString(
    const std::string& str,
    const std::string& key,
    char delimiter)
{
    std::istringstream iss(str);
    std::string token;
    while(std::getline(iss, token, delimiter))
    {
        size_t pos = token.find('=');
        if(pos != std::string::npos &&
                token.substr(0, pos) == key)
        {
            return token.substr(pos + 1);
        }
    }
    return std::nullopt;
};

/**
* @brief Helper to extract parameters from query string (after ?)
* @param query The query string (e.g. "ssl=true&timeout=10")
* @param param Parameter name to find
* @return std::optional<std::string> Extracted value or std::nullopt
*/
std::optional<std::string> DataBaseHelper::extractFromQueryString(
    const std::string& query,
    const std::string& param)
{
    std::istringstream iss(query);
    std::string token;
    while(std::getline(iss, token, '&'))
    {
        size_t pos = token.find('=');
        if(pos != std::string::npos && token.substr(0, pos) == param)
        {
            return token.substr(pos + 1);
        }
    }
    return std::nullopt;
};

/**
 * @brief Extracts a specific parameter from a database URI string.
 *
 * @param uri The connection URI string (e.g., "mysql://user:pass@host:port/dbname?options")
 * @param paramName The parameter to extract. Supported values:
 *                 - Common parameters: "host", "port", "user", "password"
 *                 - Database-specific: "dbname", "mode" (SQLite), "charset" (MySQL),
 *                   "sslmode" (PostgreSQL), "options" (MongoDB)
 * @param dbType The database type (SQLite, MySQL, PostgreSQL, MongoDB)
 * @return std::optional<std::string> Extracted value if found, std::nullopt otherwise
 *
 * @details Parses standard URI components:
 *          - scheme://[user[:password]@]host[:port][/dbname][?options]
 *          - Handles special cases for different database types
 *
 * @note For MongoDB:
 *       - "dbname" defaults to "test" if not specified
 *       - "options" returns the entire query string after '?'
 * @note For SQLite:
 *       - "dbname" returns the path portion
 *       - "mode" extracts from query parameters
 * @note Returns std::nullopt for:
 *       - Malformed URIs (missing ://)
 *       - Unsupported parameter/database combinations
 *
 * @example extractParamFromUriParam("mysql://user:pass@localhost:3306/db", "host", MySQL)
 *          returns "localhost"
 * @example extractParamFromUriParam("sqlite:///path.db?mode=rw", "mode", SQLite)
 *          returns "rw"
 */
std::optional<std::string> DataBaseHelper::extractParamFromUriParam(
    const std::string& uri,
    const std::string& paramName,
    DataBaseType dbType)
{
    if(uri.empty() || uri.find("://") == std::string::npos)
    {
        return std::nullopt;
    }

    const size_t scheme_end = uri.find("://") + 3;
    const std::string conn_part = uri.substr(scheme_end);
    const size_t at_pos = conn_part.find('@');
    const size_t slash_pos = conn_part.find('/');
    const size_t question_pos = conn_part.find('?');

    // Common parameters for all database types
    if(paramName == "host")
    {
        size_t host_start = (at_pos != std::string::npos) ? at_pos + 1 : 0;
        size_t host_end = conn_part.find_first_of(":/?", host_start);
        return conn_part.substr(host_start, host_end - host_start);
    }
    else if(paramName == "port")
    {
        size_t port_start = conn_part.find(':', (at_pos != std::string::npos) ? at_pos : 0);
        if(port_start == std::string::npos) return std::nullopt;
        size_t port_end = conn_part.find_first_of("/?", port_start);
        return conn_part.substr(port_start + 1, port_end - port_start - 1);
    }
    else if(paramName == "user")
    {
        if(at_pos == std::string::npos) return std::nullopt;
        size_t user_end = conn_part.find(':');
        if(user_end > at_pos) user_end = at_pos;
        return conn_part.substr(0, user_end);
    }
    else if(paramName == "password")
    {
        if(at_pos == std::string::npos) return std::nullopt;
        size_t pass_start = conn_part.find(':');
        if(pass_start == std::string::npos || pass_start > at_pos) return std::nullopt;
        return conn_part.substr(pass_start + 1, at_pos - pass_start - 1);
    }

    // Database-specific parameters
    switch(dbType)
    {
        case DataBaseType::SQLite:
        {
            if(paramName == "dbname")
            {
                size_t end = (question_pos != std::string::npos) ? question_pos : conn_part.length();
                return conn_part.substr(0, end);
            }
            else if(paramName == "mode")
            {
                if(question_pos == std::string::npos) return std::nullopt;
                std::string options = conn_part.substr(question_pos + 1);
                return extractFromQueryString(options, "mode");
            }
            break;
        }

        case DataBaseType::MySQL:
        {
            if(paramName == "dbname")
            {
                if(slash_pos == std::string::npos) return std::nullopt;
                size_t end = (question_pos != std::string::npos) ? question_pos : conn_part.length();
                return conn_part.substr(slash_pos + 1, end - slash_pos - 1);
            }
            else if(paramName == "charset")
            {
                if(question_pos == std::string::npos) return std::nullopt;
                std::string options = conn_part.substr(question_pos + 1);
                return extractFromQueryString(options, "charset");
            }
            break;
        }

        case DataBaseType::PostgreSQL:
        {
            if(paramName == "dbname")
            {
                if(slash_pos == std::string::npos) return std::nullopt;
                size_t end = (question_pos != std::string::npos) ? question_pos : conn_part.length();
                return conn_part.substr(slash_pos + 1, end - slash_pos - 1);
            }
            else if(paramName == "sslmode")
            {
                if(question_pos == std::string::npos) return std::nullopt;
                std::string options = conn_part.substr(question_pos + 1);
                return extractFromQueryString(options, "sslmode");
            }
            break;
        }

        case DataBaseType::MongoDB:
        {
            if(paramName == "dbname")
            {
                if(slash_pos == std::string::npos) return "test";
                size_t end = (question_pos != std::string::npos) ? question_pos : conn_part.length();
                return conn_part.substr(slash_pos + 1, end - slash_pos - 1);
            }
            else if(paramName == "options")
            {
                if(question_pos == std::string::npos) return std::nullopt;
                return conn_part.substr(question_pos + 1);
            }
            break;
        }

        default:
            return std::nullopt;
    }

    return std::nullopt;
};

/**
* @brief Extracts a specific parameter from a database connection string.
*
* @param connectionString The connection string to parse. Format varies by database type:
*                        - SQLite: Direct filename/path
*                        - MySQL: "host=...;port=...;user=...;password=...;dbname=..."
*                        - PostgreSQL: "host=... port=... user=... password=... dbname=..."
*                        - MongoDB: Either URI format ("mongodb://...") or key-value pairs
* @param paramName The name of the parameter to extract (e.g., "host", "port", "user", "password", "dbname")
* @param dbType The type of database (SQLite, MySQL, PostgreSQL, MongoDB)
* @return std::optional<std::string> The extracted parameter value if found, std::nullopt otherwise
*
* @throws std::invalid_argument If the database type is unsupported
*
* @note For SQLite:
*       - Only "dbname" parameter is supported, which returns the entire connection string
*       - Other parameter names will return std::nullopt
*
* @note For MySQL:
*       - Parses semicolon-separated key-value pairs (e.g., "host=localhost;port=3306")
*       - Parameter names are case-sensitive
*
* @note For PostgreSQL:
*       - Parses space-separated key-value pairs (e.g., "host=localhost port=5432")
*       - Parameter names are case-sensitive
*
* @note For MongoDB:
*       - First checks if the connection string is in URI format (mongodb://...)
*       - If not URI format, falls back to semicolon-separated key-value pairs
*       - Special handling for "dbname" which defaults to "test" if not specified in URI
*/
std::optional<std::string> DataBaseHelper::extractParamFromConnectionString(
    const std::string& connectionString,
    const std::string& paramName,
    DataBaseType dbType)
{
    switch(dbType)
    {
        case DataBaseType::SQLite:
            if(paramName == "dbname")
            {
                return !connectionString.empty() ?
                       std::optional(connectionString) : std::nullopt;
            }
            break;

        case DataBaseType::MySQL:
            return parseKeyValueString(connectionString, paramName, ';');

        case DataBaseType::PostgreSQL:
            return parseKeyValueString(connectionString, paramName, ' ');

        case DataBaseType::MongoDB:
            if(isUriFormat(connectionString))
            {
                return extractParamFromUriParam(connectionString, paramName, dbType);
            }
            return parseKeyValueString(connectionString, paramName, ';');

        default:
            throw std::invalid_argument(ERR_MSG_UNSUPPORTED_DB);
    }
    return std::nullopt;
};
