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

#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <memory>
#include <unordered_map>
#include <mutex>
#include <string>
#include "logger.h"
#include "log_config.h"
#include "database_factory.h"

/**
 * @class LogManager
 * @brief Singleton class for centralized logger management
 *
 * Provides thread-safe logger creation and access using configuration objects.
 * Integrates with DatabaseFactory for backend creation.
 */
class LogManager
{
    public:
        /**
         * @brief Deleted copy constructor
         */
        LogManager(const LogManager&) = delete;

        /**
         * @brief Deleted assignment operator
         */
        LogManager& operator=(const LogManager&) = delete;

        /**
         * @brief Gets the singleton instance
         * @return Reference to the LogManager instance
         */
        static LogManager& getInstance();

        /**
         * @brief Creates a new logger with specified configuration
         * @param name Unique name for the logger
         * @param config Configuration settings
         * @return Reference to the created logger
         * @throws std::runtime_error if logger name already exists
         */
        Logger& createLogger(const std::string& name, const LogConfig::Config& config
#ifdef USE_SOURCE_INFO
                             , std::optional<SourceInfo> sourceInfo = std::nullopt
#endif
                            );

        /**
         * @brief Gets an existing logger by name
         * @param name Logger name to retrieve
         * @return Reference to the logger
         * @throws std::runtime_error if logger not found
         */
        Logger& getLogger(const std::string& name);

        /**
         * @brief Removes a logger by name
         * @param name Logger name to remove
         * @throws std::runtime_error if logger not found
         */
        void removeLogger(const std::string& name);

        /**
         * @brief Removes and shuts down all managed logger instances.
         * This method performs the following operations:
         * 1. Safely shuts down each logger instance (calling shutdown() on each)
         * 2. Removes all loggers from management
         * 3. Clears the internal collection of loggers
         * @return int Number of loggers that were removed.
         * @warning After calling this method, all previously obtained logger references
         * will be invalid and should not be used.
         * @see removeLogger()
         */
        int removeAllLoggers();

        /**
         * @brief Conditionally removes logger instances based on a predicate.
         * This method removes and shuts down all logger instances that satisfy
         * the given predicate condition.
         * @tparam Predicate A callable type that accepts (const std::string&, Logger&)
         * and returns bool (true to remove, false to keep).
         * @param predicate A callable object that determines which loggers to remove.
         * @return int The number of loggers that were removed.
         * @warning Loggers removed by this method will be properly shutdown but any
         * external references to them will become invalid.
         * @see removeLogger(), removeAllLoggers()
         */
        template<typename Predicate>
        int removeIf(Predicate&& predicate)
        {
            std::lock_guard<std::mutex> lock(mutex);
            int removedCount = 0;

            for(auto it = loggers.begin(); it != loggers.end();)
            {
                if(predicate(it->first, * (it->second)))
                {
                    it->second->shutdown();
                    it = loggers.erase(it);
                    ++removedCount;
                }
                else
                {
                    ++it;
                }
            }

            return removedCount;
        };

        /**
        * @brief Gets the number of active logger instances currently managed.
        * This method returns the current count of logger instances that have been
        * created and are being managed by the LogManager.
        * @return int The number of active logger instances (always non-negative)
        * @note The count reflects only those loggers that are currently registered
        * with the manager and does not include any that may have been destroyed.
        */
        int getCount() const;

        /**
         * @brief Gets configuration for a specific logger by name.
         * @param name Logger name to look up (case sensitive)
         * @return std::optional<LogConfig::Config> Configuration if found, empty optional otherwise
         */
        std::optional<LogConfig::Config> LogManager::getLoggerConfig(const std::string& name);

        /**
         * @brief Gets all registered loggers with their configurations
         * @return std::map<std::string, LogConfig::Config> Copy of all loggers' configurations
         */
        std::map<std::string, LogConfig::Config> getAllLoggersConfigs();

    private:
        LogManager() = default;
        ~LogManager() = default;

        /**
         * @brief Creates a database instance based on configuration
         * @param config Configuration object containing database parameters. Must have:
         * - databaseType: Type of database to create
         * - Other connection parameters (host, port, etc.)
         * @return std::unique_ptr<IDatabase> Pointer to the created database instance
         * @throws std::runtime_error In following cases:
         * - If databaseType is not specified in config
         * - If connection string generation fails
         * - If database creation fails (exception propagated from DatabaseFactory)
         * @see DatabaseFactory::create()
         * @see LogConfig::configToConnectionString()
         * @see IDatabase interface for available operations
         */
        std::unique_ptr<IDatabase> createDatabase(const LogConfig::Config& config);

        std::unordered_map<std::string, std::unique_ptr<Logger>> loggers;
        std::mutex mutex;
};

#endif // LOG_MANAGER_H