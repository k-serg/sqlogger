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

#ifndef DATABASE_HELPER_H
#define DATABASE_HELPER_H

#include <string>
#include <optional>
#include "sqlogger/log_entry.h"
#include "sqlogger/log_helper.h"

#define DB_TYPE_STR_MOCK "Mock"
#define DB_TYPE_STR_SQLITE "SQLite"
#define DB_TYPE_STR_MYSQL "MySQL"
#define DB_TYPE_STR_POSTGRESQL "PostgreSQL"
#define DB_TYPE_STR_MONGODB "MongoDB"
#define DB_TYPE_STR_UNKNOWN "UNKNOWN"

#define DB_DEFAULT_PORT_NOT_SUPPORTED -1
#define DB_DEFAULT_PORT_MOCK DB_DEFAULT_PORT_NOT_SUPPORTED
#define DB_DEFAULT_PORT_SQLITE DB_DEFAULT_PORT_NOT_SUPPORTED
#define DB_DEFAULT_PORT_MYSQL 3306
#define DB_DEFAULT_PORT_POSTGRESQL 5432
#define DB_DEFAULT_PORT_MONGODB 27017

#define DB_PARAM_PREFIX_DEFAULT "?"
#define DB_PARAM_PREFIX_MOCK DB_PARAM_PREFIX_DEFAULT
#define DB_PARAM_PREFIX_SQLITE DB_PARAM_PREFIX_DEFAULT
#define DB_PARAM_PREFIX_MYSQL DB_PARAM_PREFIX_DEFAULT
#define DB_PARAM_PREFIX_POSTGRESQL "$"

#define DB_BATCH_NOT_SUPPORTED -1
#define DB_MIN_BATCH_SIZE 1
#define DB_MAX_BATCH_DEFAULT 500
#define DB_MAX_BATCH_MOCK DB_BATCH_NOT_SUPPORTED
#define DB_MAX_BATCH_SQLITE 1000
#define DB_MAX_BATCH_MYSQL 5000
#define DB_MAX_BATCH_POSTGRESQL 10000

/**
 * @enum DataBaseType
 * @brief Enumeration representing the type of database.
 */
enum class DataBaseType
{
    Unknown = -1, /**< Unknown database type. */
    Mock,         /**< Mock database type. */
    SQLite,       /**< SQLite database. */
    MySQL,        /**< MySQL database. */
    PostgreSQL,   /**< PostgreSQL database. */
    MongoDB       /**< MongoDB database. */
};

/**
 * @enum ValueType
 * @brief Determines how values should be formatted
 */
enum class ValueType
{
    Auto,    /**< Automatically detect value type */
    String,  /**< Always treat as string */
    Number   /**< Always treat as number */
};

/**
 * @namespace DataBaseHelper
 * @brief Provides utility functions for database operations
 */
namespace DataBaseHelper
{
    /**
    * @brief Checks if the database type is an embedded database
    * @details Embedded databases run in the same process as the application
    * and don't require a separate server process. Examples include SQLite.
    * @param dbType The database type to check
    * @return true If the database is embedded (runs in-process)
    * @return false If the database requires a server
    * @see isDataBaseServer()
    */
    bool isDataBaseEmbedded(const DataBaseType& dbType);

    /**
    * @brief Checks if the database type requires a server
    * @details Server databases require a separate database server process
    * and network communication. Examples include MySQL, PostgreSQL and MongoDB.
    * @param dbType The database type to check
    * @return true If the database requires a server
    * @return false If the database is embedded
    * @throw std::invalid_argument If unsupported database type is provided
    * @see isDataBaseEmbedded()
    */
    bool isDataBaseServer(const DataBaseType& dbType);

    /**
    * @brief Checks if a database type is supported in the current build configuration.
    * Determines whether the database type is available based on compile-time
    * feature flags (SQLG_USE_MYSQL, SQLG_USE_POSTGRESQL, etc.). Mock and SQLite are always
    * supported.
    * @param dbType The database type to check
    * @return bool True if the database type is supported, false otherwise
    * @see DataBaseType
    */
    bool isDataBaseSupported(const DataBaseType& dbType);

    /**
    * @brief Gets the default network port number for a specified database type.
    * Returns the standard/default port number used by the database server.
    * For database types that don't use network ports (like SQLite), returns
    * DB_DEFAULT_PORT_NOT_SUPPORTED.
    * @param dbType The database type to query
    * @return int Default port number or DB_DEFAULT_PORT_NOT_SUPPORTED if not applicable
    * @see DataBaseType
    */
    int getDataBaseDefaultPort(const DataBaseType& dbType);

    /**
     * @brief Converts a string representation of a database type to the corresponding enum value.
     * @param stringType The string representation of the database type.
     * @return The corresponding DataBaseType enum value.
     */
    DataBaseType stringToDatabaseType(const std::string& stringType);

    /**
     * @brief Converts a DataBaseType enum value to its string representation.
     * @param type The DataBaseType enum value.
     * @return The string representation of the database type.
     */
    std::string databaseTypeToString(const DataBaseType& type);

    /**
    * @brief Gets the database-specific parameter prefix string for the given database type.
    * @param type The database type to get the prefix for.
    * @return std::string The parameter prefix string specific to the database type.
    * @throw std::invalid_argument If an unsupported database type is provided.
    * @note For MongoDB, returns an empty string as it doesn't use parameter prefixes.
    * @see DB_PARAM_PREFIX_MOCK, DB_PARAM_PREFIX_SQLITE, DB_PARAM_PREFIX_MYSQL, DB_PARAM_PREFIX_POSTGRESQL.
    */
    std::string databaseTypePrefix(const DataBaseType& type);

    /**
    * @brief Gets the maximum recommended batch size for the given database type.
    * @param type The database type to get the batch size limit for.
    * @return int Maximum recommended batch size for the database type.
    * @throw std::invalid_argument If an unsupported database type is provided.
    * @note Returns DB_BATCH_NOT_SUPPORTED (-1) for Mock and MongoDB databases (no batching supported).
    * @see DB_MAX_BATCH_SQLITE, DB_MAX_BATCH_MYSQL, DB_MAX_BATCH_POSTGRESQL.
    */
    int getMaxBatchSize(const DataBaseType& type);

    /**
     * @brief Escapes backslashes in a string.
     * @param input The input string to escape.
     * @return The string with escaped backslashes.
     */
    std::string escapeBackslashes(const std::string& input);

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
    bool isUriFormat(const std::string& connectionString);

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
    std::optional<std::string> parseKeyValueString(
        const std::string& str,
        const std::string& key,
        char delimiter);

    /**
    * @brief Helper to extract parameters from query string (after ?)
    * @param query The query string (e.g. "ssl=true&timeout=10")
    * @param param Parameter name to find
    * @return std::optional<std::string> Extracted value or std::nullopt
    */
    std::optional<std::string> extractFromQueryString(
        const std::string& query,
        const std::string& param);

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
    std::optional<std::string> extractParamFromUriParam(
        const std::string& uri,
        const std::string& paramName,
        DataBaseType dbType);

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
    std::optional<std::string> extractParamFromConnectionString(
        const std::string& connectionString,
        const std::string& paramName,
        DataBaseType dbType);
};

#endif // !DATABASE_HELPER_H
