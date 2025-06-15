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
#include "sqlogger/log_entry.h"
#include "sqlogger/log_helper.h"
#include "sqlogger/database/database_helper.h"
#include "sqlogger/log_crypto.h"
#include "sqlogger/transport/transport_helper.h"

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
#define LOG_INI_KEY_DATABASE_NAME "Name"
#define LOG_INI_KEY_DATABASE_TABLE "Table"
#define LOG_INI_KEY_DATABASE_HOST "Host"
#define LOG_INI_KEY_DATABASE_PORT "Port"
#define LOG_INI_KEY_DATABASE_USER "User"
#define LOG_INI_KEY_DATABASE_PASS "Pass"
#define LOG_INI_KEY_DATABASE_TYPE "Type"

#ifdef SQLG_USE_SOURCE_INFO
    #define LOG_INI_SECTION_SOURCE "Source"
    #define LOG_INI_KEY_SOURCE_UUID "Uuid"
    #define LOG_INI_KEY_SOURCE_NAME "Name"
#endif

#define LOG_INI_SECTION_TRANSPORT "Transport"
#define LOG_INI_KEY_TRANSPORT_TYPE "Type"
#define LOG_INI_KEY_TRANSPORT_HOST "Host"
#define LOG_INI_KEY_TRANSPORT_PORT "Port"

#define CON_STR_HOST LOG_INI_KEY_DATABASE_HOST
#define CON_STR_PORT LOG_INI_KEY_DATABASE_PORT
#define CON_STR_DB LOG_INI_KEY_DATABASE_NAME
#define CON_STR_USER LOG_INI_KEY_DATABASE_USER
#define CON_STR_PASS LOG_INI_KEY_DATABASE_PASS

#define LOG_NUM_THREADS_MIN 1
#define LOG_NUM_THREADS_MAX 256
#define LOG_MIN_PORT_NUM 0
#define LOG_MAX_PORT_NUM 65535

constexpr char* LOG_DEFAULT_INI_FILENAME = SQLOGGER_PROJECT_NAME ".ini";

constexpr char* tagLogger = "[" LOG_INI_SECTION_LOGGER "]";
constexpr char* tagDatabase = "[" LOG_INI_SECTION_DATABASE "]";

#ifdef SQLG_USE_SOURCE_INFO
    constexpr char* tagSource = "[" LOG_INI_SECTION_SOURCE "]";
#endif

/**
 *Used to validate user input and configuration parameters before using them in SQL queries.
 * @note Patterns are :
 * - SQL comments(--)
 * - Statement terminators(;)
 * - String delimiters(", ')
 * - Dangerous SQL keywords(SELECT, INSERT, DROP etc.)
 * - Common injection patterns(1 = 1, ' OR '1'='1)
 * @see containsSQLInjection()
 * @see validateDatabase()
 */
constexpr const char* dangerousSQLPatterns[] =
{
    "--", ";", "\"", "\'",
    "/*", "*/", "xp_", "exec ",
    "union ", "select ", "insert ",
    "update ", "delete ", "drop ",
    "truncate ", "alter ", "create ",
    "shutdown", "1=1", " or "
};

/**
 * @namespace LogConfig
 * @brief Provides configuration management for the logging system
 * @details This namespace contains all types and functions related to logger configuration,
 * including loading/saving settings from INI files, managing database connections,
 * and handling security-sensitive parameters.
 * @see Config for the main configuration structure
 * @see DataBaseType for supported database types
 * @see INI namespace for file format details
 */
namespace LogConfig
{
    /**
     * @struct ValidateResult
     * @brief Container for configuration validation results.
     * Stores validation status including missing and invalid parameters with error details.
     * Provides methods to combine results and format error output.
     */
    struct ValidateResult
    {
        public:

            /**
            * @brief Checks if validation was successful.
            * @return true if all validations passed (no missing or invalid parameters).
            * @return false if any validation failed.
            */
            inline bool ok() const;

            /**
            * @brief Formats validation errors as human-readable string.
            * @return std::string Formatted error message containing:
            * - Missing parameters (comma-separated).
            * - Invalid parameters with details (one per line).
            */
            std::string print() const;

        private:

            friend class Config;

            /**
            * @brief Records a missing parameter error.
            * @param param Name of the missing parameter.
            * @note Automatically sets success=false.
            */
            inline void addMissing(const::std::string& param);

            /**
            * @brief Records an invalid parameter error.
            * @param param Name of the invalid parameter.
            * @param details Description of the validation failure.
            * @note Automatically sets success=false.
            */
            inline void addInvalid(const::std::string& param, const::std::string& details);

            /**
            * @brief Combines validation results from another ValidateResult.
            * @param other Another validation result to merge into this one.
            * @note Merged result will be unsuccessful if either source was unsuccessful.
            */
            void merge(const ValidateResult& other);

            bool success = true; ///< Overall validation status (true if all checks passed)
            std::vector<std::string> missingParams; ///< List of missing required parameters
            std::vector<std::pair<std::string, std::string>> invalidParams; ///< List of invalid parameters with error details
    };

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
            std::optional<TransportType> transportType;
            std::optional<std::string> transportHost;
            std::optional<int> transportPort;

#ifdef SQLG_USE_SOURCE_INFO
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

            /**
             * @brief Validates the entire configuration
             * @return ValidateResult Contains validation status and error details:
             * - success: true if all validations pass
             * - missingParams: List of missing required parameters
             * - invalidParams: List of invalid parameters with error messages
             * @note This method combines results from all specific validators (name, database, etc.)
             * @see validateName(), validateDatabase(), validateThreads()
             * @see validateSource(), validateBatch(), validateLogLevel()
             */
            ValidateResult validate() const;

            /**
             * @brief Validates the entire configuration (static method)
             * @return ValidateResult Contains validation status and error details:
             * - success: true if all validations pass
             * - missingParams: List of missing required parameters
             * - invalidParams: List of invalid parameters with error messages
             * @note This method combines results from all specific validators (name, database, etc.)
             * @see validateName(), validateDatabase(), validateThreads()
             * @see validateSource(), validateBatch(), validateLogLevel()
             */
            static ValidateResult validate(const Config& config);

        private:

            /**
             * @brief Checks for potential SQL injection patterns in input string
             * @param input The string to validate
             * @return true if dangerous patterns detected, false otherwise
             * @note Uses case-insensitive comparison via LogHelper::toLowerCase()
             */
            bool containsSQLInjection(const std::string& input) const;

            /**
             * @brief Validates the logger name configuration
             * @return ValidateResult Contains:
             * - success: true if name is valid
             * - missingParams: Contains "name" if empty/missing
             * - invalidParams: Empty (no invalid state possible for name)
             * @details Checks:
             * - Name is not empty (if present)
             * - Name meets length requirements (if any)
             */
            ValidateResult validateName() const;

            /**
             * @brief Validates database connection parameters
             * @return ValidateResult Contains:
             * - success: true if all database parameters are valid
             * - missingParams: List of missing required database params
             * - invalidParams: List of invalid database params with error details
             * @details Checks:
             * - Database type is supported
             * - Required parameters for the database type are present
             * - Port number is valid (0-65535)
             * - Credentials are provided (if required)
             * - Table name is specified
             */
            ValidateResult validateDatabase() const;

            /**
             * @brief Validates threading configuration
             * @return ValidateResult Contains:
             * - success: true if thread configuration is valid
             * - missingParams: Empty (thread count has default value)
             * - invalidParams: Contains error if thread count is out of valid range
             * @details Checks:
             * - Thread count is within allowed range (1-256)
             * - Thread count is present if async mode is enabled
             */
            ValidateResult validateThreads() const;

            /**
             * @brief Validates batch configuration
             * @return ValidateResult Contains:
             * - success: true if batch configuration is valid
             * - missingParams: Empty (batch size has default value)
             * - invalidParams: Contains error if batch size is out of valid range
             * @details Checks:
             * - Batch size is within allowed range for database type (1-10000 depends on database type)
             * - Batch size is present if async mode is enabled
             * @see getMaxBatchSize()
             * @see DB_MAX_BATCH_SQLITE, DB_MAX_BATCH_MYSQL, DB_MAX_BATCH_POSTGRESQL
             */
            ValidateResult validateBatch() const;

            /**
             * @brief Validates the minimum log level configuration
             * @return ValidateResult Contains validation status with:
             * - success: true if log level is properly configured
             * - missingParams: Contains log level key if not specified
             * - invalidParams: Empty (no invalid state possible for log level)
             * @details Checks:
             * - Minimum log level is specified in configuration
             * - Log level value is within allowed range (implied by LogLevel enum)
             * @note The actual validity of the log level value is ensured by
             * LogHelper::stringToLevel() during configuration loading.
             * This method only verifies presence of the setting.
             * @see LOG_INI_KEY_MIN_LOG_LEVEL
             * @see LogLevel
             * @see LogHelper::stringToLevel()
             */
            ValidateResult validateLogLevel() const;

#ifdef SQLG_USE_SOURCE_INFO
            /**
             * @brief Validates source UUID configuration (if enabled)
             * @return ValidateResult Contains:
             * - success: true if UUID is valid
             * - missingParams: Contains "uuid" if missing when required
             * - invalidParams: Contains "uuid" if format is invalid
             * @details Checks:
             * - UUID is present (if source info is enabled)
             * - UUID follows proper format (if specified)
             * @note Only active when SQLG_USE_SOURCE_INFO is defined
             */
            ValidateResult validateSource() const;
#endif
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