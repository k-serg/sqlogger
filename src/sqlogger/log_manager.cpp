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

#include "sqlogger/log_manager.h"

/**
 * @brief Gets the singleton instance
 * @return Reference to the LogManager instance
 */
LogManager& LogManager::getInstance()
{
    static LogManager instance;
    return instance;
}

/**
 * @brief Creates a new logger with specified configuration
 * @param name Unique name for the logger
 * @param config Configuration settings
 * @return Reference to the created logger
 * @throws std::runtime_error if logger name already exists
 */
SQLogger& LogManager::createLogger(const std::string& name,
                                   const LogConfig::Config& config
#ifdef SQLG_USE_SOURCE_INFO
    , std::optional<SourceInfo> sourceInfo
#endif
                                  )
{
    std::lock_guard<std::mutex> lock(mutex);

    if(loggers.count(name))
    {
        throw std::runtime_error(ERR_MSG_LOGGER_EXISTS + name);
    }

    if(!config.databaseType)
    {
        throw std::runtime_error(ERR_MSG_DB_TYPE_NOT_SPECIFIED);
    }

    std::string connectionString = LogConfig::configToConnectionString(config);
    auto db = DatabaseFactory::create( * config.databaseType, connectionString);

    loggers[name] = std::unique_ptr<SQLogger>(new SQLogger(std::move(db), config
#ifdef SQLG_USE_SOURCE_INFO
                    , std::move(sourceInfo)
#endif
                                                          ));
    return *loggers[name];
}

/**
 * @brief Gets an existing logger by name
 * @param name Logger name to retrieve
 * @return Reference to the logger
 * @throws std::runtime_error if logger not found
 */
SQLogger& LogManager::getLogger(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = loggers.find(name);
    if(it == loggers.end())
    {
        std::string errorMsg = ERR_MSG_LOGGER_NAME_NOT_FOUND + name + " " + ERR_MSG_AVAILABLE_LOGGERS;

        std::vector<std::string> availableLoggers;
        for(const auto & pair : loggers)
        {
            availableLoggers.push_back(pair.first);
        }

        if(!availableLoggers.empty())
        {
            errorMsg += StringHelper::join(availableLoggers, ", ");
        }
        else
        {
            errorMsg += "none";
        }

        throw std::runtime_error(errorMsg);
    }

    return *(it->second);
}

/**
 * @brief Removes a logger by name
 * @param name Logger name to remove
 * @throws std::runtime_error if logger not found
 */
void LogManager::removeLogger(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex);
    auto it = loggers.find(name);
    if(it != loggers.end())
    {
        it->second->shutdown();
        loggers.erase(it);
    }
    else
    {
        throw std::runtime_error(ERR_MSG_LOGGER_NAME_NOT_FOUND + name);
    }
}

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
int LogManager::removeAllLoggers()
{
    std::lock_guard<std::mutex> lock(mutex);
    const int count = loggers.size();
    for(auto & [name, logger] : loggers)
    {
        logger->shutdown();
    }
    loggers.clear();
    return count;
}

/**
 * @brief Gets the number of active logger instances currently managed.
 * This method returns the current count of logger instances that have been
 * created and are being managed by the LogManager.
 * @return int The number of active logger instances (always non-negative)
 * @note The count reflects only those loggers that are currently registered
 * with the manager and does not include any that may have been destroyed.
 */
int LogManager::getCount() const
{
    return loggers.size();
}

/**
 * @brief Gets configuration for a specific logger by name.
 * @param name Logger name to look up (case sensitive).
 * @return std::optional<LogConfig::Config> Configuration if found, empty optional otherwise.
 */
std::optional<LogConfig::Config> LogManager::getLoggerConfig(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex);
    if(auto it = loggers.find(name); it != loggers.end())
    {
        return it->second->getConfig();
    }
    return std::nullopt;
}

/**
 * @brief Gets all registered loggers with their configurations
 * @return std::map<std::string, LogConfig::Config> Copy of all loggers' configurations
 */
std::map<std::string, LogConfig::Config> LogManager::getAllLoggersConfigs()
{
    std::lock_guard<std::mutex> lock(mutex);
    std::map<std::string, LogConfig::Config> result;
    for(const auto & [name, logger] : loggers)
    {
        result.emplace(name, logger->getConfig());
    }
    return result;
}

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
std::unique_ptr<IDatabase> LogManager::createDatabase(const LogConfig::Config& config)
{
    if(!config.databaseType)
    {
        throw std::runtime_error(ERR_MSG_DB_TYPE_NOT_SPECIFIED);
    }

    std::string connStr = LogConfig::configToConnectionString(config);
    return DatabaseFactory::create( * config.databaseType, connStr);
}
