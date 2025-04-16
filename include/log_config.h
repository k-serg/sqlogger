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
#define LOG_NUM_THREADS 4 ///< Default number of threads for asynchronous logging.
#define LOG_SYNC_MODE 1 ///< Default synchronization mode (true for synchronous logging).
#define LOG_ONLY_FILE_NAMES 0 ///< Default whether to log only filenames (without full paths).
constexpr LogLevel LOG_MIN_LOG_LEVEL = LogLevel::Trace; ///< Default minimum log level for messages to be logged.

#define LOG_INI_SECTION_LOGGER "Logger"
#define LOG_INI_KEY_SYNC_MODE "SyncMode"
#define LOG_INI_KEY_NUM_THREADS "NumThreads"
#define LOG_INI_KEY_ONLY_FILE_NAMES "OnlyFileNames"
#define LOG_INI_KEY_MIN_LOG_LEVEL "MinLogLevel"

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

constexpr char* LOG_INI_FILENAME = SQLOGGER_PROJECT_NAME ".ini";

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
    static std::string join(const std::vector<std::string> & parts, const std::string& delimiter)
    {
        std::string result;
        for(size_t i = 0; i < parts.size(); ++i)
        {
            if(i != 0)
            {
                result += delimiter;
            }
            result += parts[i];
        }
        return result;
    }
};

namespace LogConfig
{
    /**
     * @struct Config
     * @brief Configuration settings for the logger.
     */
    struct Config
    {
        std::optional<bool> syncMode = LOG_SYNC_MODE; ///< Synchronization mode (true for synchronous logging).
        std::optional<size_t> numThreads = LOG_NUM_THREADS; ///< Number of threads for asynchronous logging.
        std::optional<bool> onlyFileNames = LOG_ONLY_FILE_NAMES; ///< Whether to log only filenames (without full paths).
        std::optional<LogLevel> minLogLevel = LOG_MIN_LOG_LEVEL; ///< Minimum log level for messages to be logged.
        std::optional<std::string> databaseName; ///< Name of the database to use for logging.
        std::optional<std::string> databaseTable; ///< Name of the table to use for logging.
        std::optional<std::string> databaseHost; ///< Host address of the database.
        std::optional<int> databasePort; ///< Port number of the database.
        std::optional<std::string> databaseUser; ///< Username for the database.
        std::optional<std::string> databasePass; ///< Password for the database.
        std::optional<DataBaseType> databaseType; ///< Type of the database (e.g., MySQL, SQLite).
#ifdef USE_SOURCE_INFO
        std::optional<std::string> sourceUuid;  /**< The universally unique identifier (UUID) of the source. */
        std::optional<std::string> sourceName;  /**< The name of the source. */
#endif
        std::optional<std::string> passKey; ///< Key for password encryption and decryption.

        void setPassKey(const std::string& passKey)
        {
            this->passKey = passKey;
        };

        std::string getPassKey()
        {
            return passKey.value();
        };

        /**
         * @brief Loads configuration from an INI file.
         * @param filename The path to the INI file.
         * @return A Config object containing the loaded settings.
         * @throws std::runtime_error If the file cannot be parsed.
         */
        static Config loadFromINI(const std::string& filename = LOG_INI_FILENAME, const std::string& passKey = "");

        /**
         * @brief Saves configuration to an INI file.
         * @param config The Config object to save.
         * @param filename The path to the INI file.
         * @throws std::runtime_error If the file cannot be written.
         */
        static void saveToINI(const Config& config, const std::string& filename = LOG_INI_FILENAME);
    };


    static std::string configToConnectionString(const LogConfig::Config& config)
    {
        if(!config.databaseType.has_value())
        {
            throw std::runtime_error("Database type is not specified in config");
        }

        switch(config.databaseType.value())
        {
            case DataBaseType::Mock:
                return config.databaseName.value_or("");
                break;

            case DataBaseType::SQLite:
                return config.databaseName.value_or("");
                break;

            case DataBaseType::MySQL:
            {
                std::vector<std::string> parts;
                if(config.databaseHost.has_value())
                    parts.push_back(std::string(CON_STR_HOST) + "=" + config.databaseHost.value());
                if(config.databaseUser.has_value())
                    parts.push_back(std::string(CON_STR_USER) + "=" + config.databaseUser.value());
                if(config.databasePass.has_value())
                    parts.push_back(std::string(CON_STR_PASS) + "=" + config.databasePass.value());
                if(config.databaseName.has_value())
                    parts.push_back(std::string(CON_STR_DB) + "=" + config.databaseName.value());
                if(config.databasePort.has_value())
                    parts.push_back(std::string(CON_STR_PORT) + "=" + std::to_string(config.databasePort.value()));

                return StringHelper::join(parts, ";");
            }
            break;

            case DataBaseType::PostgreSQL:
            {
                std::vector<std::string> parts;
                if(config.databaseHost.has_value())
                    parts.push_back("host=" + config.databaseHost.value());
                if(config.databaseUser.has_value())
                    parts.push_back("user=" + config.databaseUser.value());
                if(config.databasePass.has_value())
                    parts.push_back("password=" + config.databasePass.value());
                if(config.databaseName.has_value())
                    parts.push_back("dbname=" + config.databaseName.value());
                if(config.databasePort.has_value())
                    parts.push_back("port=" + std::to_string(config.databasePort.value()));

                return StringHelper::join(parts, " ");
            }
            break;

            case DataBaseType::MongoDB:
            {
                std::vector<std::string> parts;
                if(config.databaseHost.has_value())
                    parts.push_back("host=" + config.databaseHost.value());
                if(config.databaseUser.has_value())
                    parts.push_back("user=" + config.databaseUser.value());
                if(config.databasePass.has_value())
                    parts.push_back("password=" + config.databasePass.value());
                if(config.databaseName.has_value())
                    parts.push_back("database=" + config.databaseName.value());
                if(config.databasePort.has_value())
                    parts.push_back("port=" + std::to_string(config.databasePort.value()));

                return StringHelper::join(parts, ";");
            }
            break;

            default:
                throw std::runtime_error(ERR_MSG_UNSUPPORTED_DB);
        }
    };

};

#endif // !LOG_CONFIG_H