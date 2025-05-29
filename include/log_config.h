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

#ifndef LOG_CONFIG_H
#define LOG_CONFIG_H

#include <optional>
#include <string>
#include "sqlogger_config.h"
#include "log_entry.h"
#include "database_helper.h"
#include "log_crypto.h"

// Defaults
#define LOG_DEFAULT_LOGGER_NAME "Default" ///< Default logger name.
#define LOG_DEFAULT_NUM_THREADS 4 ///< Default number of threads for asynchronous logging.
#define LOG_DEFAULT_SYNC_MODE 1 ///< Default synchronization mode (true for synchronous logging).
#define LOG_DEFAULT_ONLY_FILE_NAMES 0 ///< Default whether to log only filenames (without full paths).
constexpr LogLevel LOG_DEFAULT_MIN_LOG_LEVEL = LogLevel::Trace; ///< Default minimum log level for messages to be logged.

#define LOG_INI_SECTION_LOGGER "Logger"
#define LOG_INI_KEY_NAME "Name"
#define LOG_INI_KEY_SYNC_MODE "SyncMode"
#define LOG_INI_KEY_NUM_THREADS "NumThreads"
#define LOG_INI_KEY_ONLY_FILE_NAMES "OnlyFileNames"
#define LOG_INI_KEY_MIN_LOG_LEVEL "MinLogLevel"
#define LOG_INI_KEY_USE_BATCH "UseBatch"
#define LOG_INI_KEY_BATCH_SIZE "BatchSize"

#define LOG_INI_SECTION_DATABASE "Database"
#define LOG_INI_KEY_DATABASE_NAME "Database"
#define LOG_INI_KEY_DATABASE_TABLE "Table"
#define LOG_INI_KEY_DATABASE_HOST "Host"
#define LOG_INI_KEY_DATABASE_PORT "Port"
#define LOG_INI_KEY_DATABASE_USER "User"
#define LOG_INI_KEY_DATABASE_PASS "Pass"
#define LOG_INI_KEY_DATABASE_TYPE "Type"

#ifdef USE_SOURCE_INFO
    #define LOG_INI_SECTION_SOURCE "Source"
    #define LOG_INI_KEY_SOURCE_UUID "Uuid"
    #define LOG_INI_KEY_SOURCE_NAME "Name"
#endif

#define CON_STR_HOST LOG_INI_KEY_DATABASE_HOST
#define CON_STR_PORT LOG_INI_KEY_DATABASE_PORT
#define CON_STR_DB LOG_INI_KEY_DATABASE_NAME
#define CON_STR_USER LOG_INI_KEY_DATABASE_USER
#define CON_STR_PASS LOG_INI_KEY_DATABASE_PASS

constexpr char* LOG_DEFAULT_INI_FILENAME = SQLOGGER_PROJECT_NAME ".ini";

/**
 * @namespace StringHelper
 * @brief Provides utility functions for string operations
 */
namespace StringHelper
{
    /**
     * @brief Joins a vector of strings into a single string with a delimiter
     * @param parts The vector of strings to join
     * @param delimiter The string to insert between joined parts
     * @return std::string The resulting concatenated string
     */
    std::string join(const std::vector<std::string> & parts, const std::string& delimiter);
};

/**
 * @namespace LogConfig
 * @brief Provides configuration management for the logging system
 *
 * @details This namespace contains all types and functions related to logger configuration,
 * including loading/saving settings from INI files, managing database connections,
 * and handling security-sensitive parameters.
 *
 * @see Config for the main configuration structure
 * @see DataBaseType for supported database types
 * @see INI namespace for file format details
 */
namespace LogConfig
{
    /**
     * @struct Config
     * @brief Configuration settings for the logger.
     */
    struct Config
    {
        std::optional<std::string> name; ///< Logger name.
        std::optional<bool> syncMode; ///< Synchronization mode (true for synchronous logging).
        std::optional<size_t> numThreads; ///< Number of threads for asynchronous logging.
        std::optional<bool> onlyFileNames; ///< Whether to log only filenames (without full paths).
        std::optional<LogLevel> minLogLevel; ///< Minimum log level for messages to be logged.
        std::optional<std::string> databaseName; ///< Name of the database to use for logging.
        std::optional<std::string> databaseTable; ///< Name of the table to use for logging.
        std::optional<std::string> databaseHost; ///< Host address of the database.
        std::optional<int> databasePort; ///< Port number of the database.
        std::optional<std::string> databaseUser; ///< Username for the database.
        std::optional<std::string> databasePass; ///< Password for the database.
        std::optional<DataBaseType> databaseType; ///< Type of the database (e.g., MySQL, SQLite).
        std::optional<bool> useBatch;
        std::optional<int> batchSize;
#ifdef USE_SOURCE_INFO
        std::optional<std::string> sourceUuid;  /**< The universally unique identifier (UUID) of the source. */
        std::optional<std::string> sourceName;  /**< The name of the source. */
#endif
        std::optional<std::string> passKey; ///< Key for password encryption and decryption.

        /**
        * @brief Sets the password key for password encryption and decryption.
        * @param passKey The secret key for password encryption and decryption.
        * @throws std::runtime_error If passKey empty.
        * @note The key is stored in memory in plaintext. Consider additional security
        * measures if handling highly sensitive data.
        * @warning Avoid hardcoding keys in source code. Prefer secure configuration
        * or key management systems for production environments.
        */
        void setPassKey(const std::string& passKey);

        /**
        * @brief Retrieves the current password key
        * @return std::string The stored password key
        * @throws std::runtime_error If no key has been set (when passKey is empty)
        * @note The returned key should be handled securely and cleared from memory
        * when no longer needed.
        * @warning Exposing the key through logs or debugging outputs creates
        * security vulnerabilities.
        */
        std::string getPassKey();

        /**
         * @brief Loads configuration from an INI file.
         * @param filename The path to the INI file.
         * @param passKey The key for password decryption.
         * @return A Config object containing the loaded settings.
         * @throws std::runtime_error If the file cannot be parsed.
         */
        static Config loadFromINI(const std::string& filename = LOG_DEFAULT_INI_FILENAME, const std::string& passKey = "");

        /**
         * @brief Saves configuration to an INI file.
         * @param config The Config object to save.
         * @param filename The path to the INI file.
         * @throws std::runtime_error If the file cannot be written.
         */
        static void saveToINI(const Config& config, const std::string& filename = LOG_DEFAULT_INI_FILENAME);
    };

    /**
    * @brief Converts a LogConfig::Config object into a database-specific connection string
    * @param config Configuration object containing database connection parameters
    * @return std::string Formatted connection string appropriate for the specified database type
    *
    * @throws std::runtime_error If:
    *         - Database type is not specified in config
    *         - Unsupported database type is requested
    *
    * @note Connection string format varies by database type:
    *       - Mock/SQLite: Returns just the database name (or empty string)
    *       - MySQL: "host=value;user=value;password=value" format
    *       - PostgreSQL: "host=value user=value password=value" format (space-separated)
    *       - MongoDB: "host=value;user=value;password=value" format
    *
    * @warning All parameters are optional except databaseType. Missing parameters will be omitted
    *          from the connection string which may cause connection failures.
    *
    * @see LogConfig::Config
    * @see DataBaseType
    * @see StringHelper::join()
    */
    std::string configToConnectionString(const Config& config);
};

#endif // !LOG_CONFIG_H