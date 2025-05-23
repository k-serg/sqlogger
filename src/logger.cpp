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

#include "logger.h"

/**
 * @brief Constructs a Logger object.
 * @param database The database interface to use for logging.
 * @param config LogConfig::Config struct that will be applied.
 */
Logger::Logger(std::unique_ptr<IDatabase> database, const LogConfig::Config& config
#ifdef USE_SOURCE_INFO
    , std::optional<SourceInfo> sourceInfo
#endif
              )
    : database(std::move(database)),
      writer( * this->database),
      reader( * this->database),
      threadPool(config.numThreads.value_or(LOG_NUM_THREADS)),
      running(true),
      totalTasksProcessed(0),
      totalProcessingTime(0),
      maxProcessingTime(0),
      minLevel(config.minLogLevel.value_or(LOG_MIN_LOG_LEVEL)),
      syncMode(config.syncMode.value_or(LOG_SYNC_MODE)),
      onlyFileNames(config.onlyFileNames.value_or(LOG_ONLY_FILE_NAMES))
#ifdef USE_SOURCE_INFO
    , sourceInfo(sourceInfo)
    , sourceId(sourceInfo.has_value() && sourceInfo.value().sourceId != SOURCE_NOT_FOUND ? sourceInfo.value().sourceId : SOURCE_NOT_FOUND)
#endif
{
    std::scoped_lock lock(dbMutex);

#ifdef USE_SOURCE_INFO

    std::scoped_lock sourceLock(sourceMutex);

    // Create sources table first!
    writer.createSourcesTable();

    if(this->sourceInfo.has_value())
    {
        // Get existing source with uuid.
        auto storedSource = reader.getSourceByUuid(this->sourceInfo.value().uuid);

        // Compare existing source uuid with this uuid.
        if(storedSource.has_value() && storedSource.value().uuid == this->sourceInfo.value().uuid)
        {
            this->sourceId = storedSource.value().sourceId;
            this->sourceInfo = storedSource;
        }
        else
        {
            // Or add new source.
            this->sourceId = writer.addSource(this->sourceInfo->name, this->sourceInfo->uuid);
            this->sourceInfo = reader.getSourceById(this->sourceId);
        }
    }
    else if(config.sourceUuid.has_value() && config.sourceName.has_value() && !config.sourceUuid->empty() && !config.sourceName->empty())
    {
        // Get existing source with config uuid.
        auto storedSource = reader.getSourceByUuid(config.sourceUuid.value());

        // Compare existing source uuid with config uuid.
        if(storedSource.has_value() && storedSource.value().uuid == config.sourceUuid.value())
        {
            this->sourceId = storedSource.value().sourceId;
            this->sourceInfo = storedSource;
        }
        else
        {
            // Or add new source.
            this->sourceId = writer.addSource(this->sourceInfo->name, this->sourceInfo->uuid);
            this->sourceInfo = reader.getSourceById(this->sourceId);
        }
    }
    else
    {
        // Or add new source with default name
        this->sourceId = writer.addSource(SOURCE_DEFAULT_NAME);
        this->sourceInfo = reader.getSourceById(this->sourceId);
    }
#endif

    writer.createLogsTable();
    writer.createIndexes();
}

/**
 * @brief Destructor for Logger. Stops all threads and releases resources.
 */
Logger::~Logger()
{
    shutdown();
}

/**
 * @brief Shuts down the logger and stops all worker threads.
 */
void Logger::shutdown()
{
    running = false;
    if(!syncMode)
    {
        threadPool.waitForCompletion();
    }

    if(database)
    {
        database.reset();
    }
}

/**
* @brief Logs a message with the specified level and details.
* @param level The severity level of the log message.
* @param message The log message.
*/
void Logger::log(const LogLevel level, const std::string& message)
{
    logAdd(level, message, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()));
}

/**
 * @brief Logs a message with the specified level and details.
 * @param level The severity level of the log message.
 * @param message The log message.
 * @param function The function where the log message was created.
 * @param file The file where the log message was created.
 * @param line The line number where the log message was created.
 * @param threadId The ID of the thread that created the log message.
 */
void Logger::logAdd(const LogLevel level, const std::string& message, const std::string& function, const std::string& file, int line, const std::string& threadId)
{
    if(level < minLevel) return;

    std::string fileName(file);
    if(onlyFileNames)
    {
        fileName = std::filesystem::path(fileName).filename().string();
    }

#ifdef USE_SOURCE_INFO
    if(sourceId == SOURCE_NOT_FOUND)
    {
        logError(ERR_MSG_SOURCE_ID_NOT_INIT);
        return;
    }
#endif

    if(syncMode)
    {
        LogTask task{ level, message, function, fileName, line, threadId, std::chrono::system_clock::now()
#ifdef USE_SOURCE_INFO
                      , sourceId
#endif
                    };
        processTask(task);
    }
    else
    {
        threadPool.enqueue([this, level, message, function, fileName, line, threadId]
        {
            LogTask task{
                level, message, function, fileName, line, threadId, std::chrono::system_clock::now()
#ifdef USE_SOURCE_INFO
                , sourceId
#endif
            };
            processTask(task);
        });
    }
}

/**
 * @brief Waits until the task queue is empty.
 * @param timeout The maximum time to wait.
 * @return True if the queue is empty within the timeout, false otherwise.
 */
bool Logger::waitUntilEmpty(const std::chrono::milliseconds& timeout)
{
    if(syncMode)
    {
        return true;
    }

    auto start = std::chrono::steady_clock::now();
    while(!threadPool.isQueueEmpty())
    {
        if(std::chrono::steady_clock::now() - start > timeout)
        {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return true;
}

/**
* @brief Exports log entries to a specified format file.
* @param filePath The path to the output file.
* @param format Format of the output file.
* @param entryList The list of log entries to export.
* @param delimiter The delimiter to use between fields.
* @param name Whether to include field names in the output.
*/
void Logger::exportTo(const std::string& filePath, const LogExport::Format& format, const LogEntryList& entryList, const std::string& delimiter, bool name)
{
    LogExport::exportTo(filePath, format, entryList, delimiter, name);
}


/**
 * @brief Processes a single log task.
 * @param task The log task to process.
 */
void Logger::processTask(const LogTask& task)
{
    try
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        {
            std::scoped_lock lock(dbMutex, statsMutex);
            std::string levelStr = levelToString(task.level);
            std::string timestamp = getCurrentTimestamp();

            LogEntry entry
            {
                0,
#ifdef USE_SOURCE_INFO
                task.sourceId,
#endif
                timestamp,
                levelStr,
                task.message,
                task.function,
                task.file,
                task.line,
                task.threadId
#ifdef USE_SOURCE_INFO
                , "", // uuid
                "" // sourceName
#endif
            };

#ifdef USE_SOURCE_INFO
            if(task.sourceId != SOURCE_NOT_FOUND)
            {
                auto sourceInfo = reader.getSourceById(task.sourceId);
                if(sourceInfo)
                {
                    entry.uuid = sourceInfo->uuid;
                    entry.sourceName = sourceInfo->name;
                }
                else
                {
                    logError(ERR_MSG_SOURCE_NOT_FOUND + std::to_string(task.sourceId));
                }
            }
#endif
            if(!writer.writeLog(entry))
            {
                logError(ERR_MSG_FAILED_QUERY);
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            double taskTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();

            totalTasksProcessed++;
            totalProcessingTime += taskTime;
            if(taskTime > maxProcessingTime)
            {
                maxProcessingTime = taskTime;
            }
        }
    }
    catch(const std::exception& e)
    {
        logError(ERR_MSG_FAILED_TASK + std::string(e.what()));
    }
}

/**
 * @brief Processes a batch of log tasks.
 * @param batch The batch of log tasks to process.
 */
void Logger::processBatch(const std::vector<LogTask> & batch)
{
    try
    {
        auto startTime = std::chrono::high_resolution_clock::now();
        {
            std::scoped_lock lock(dbMutex, statsMutex);
            for(const auto & task : batch)
            {
                std::string levelStr = levelToString(task.level);
                std::string timestamp = getCurrentTimestamp();

                LogEntry entry
                {
                    0,  // id
#ifdef USE_SOURCE_INFO
                    task.sourceId,
#endif
                    timestamp,
                    levelStr,
                    task.message,
                    task.function,
                    task.file,
                    task.line,
                    task.threadId
#ifdef USE_SOURCE_INFO
                    , "", // uuid
                    "" // sourceName
#endif
                };

                if(!writer.writeLog(entry))
                {
                    logError(ERR_MSG_FAILED_QUERY);
                }
            }

            auto endTime = std::chrono::high_resolution_clock::now();
            double taskTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();

            totalTasksProcessed += batch.size();
            totalProcessingTime += taskTime;
            if(taskTime > maxProcessingTime)
            {
                maxProcessingTime = taskTime;
            }
        }
    }
    catch(const std::exception& e)
    {
        logError("Error in processBatch: " + std::string(e.what()));
    }
}

/**
 * @brief Logs an error message to the error log file.
 * @param errorMessage The error message to log.
 */
void Logger::logError(const std::string& errorMessage)
{
    std::ofstream errorLog(ERR_LOG_FILE, std::ios::app);
    if(!errorLog.is_open())
    {
        std::cerr << ERR_MSG_FAILED_OPEN_ERR_LOG << ERR_LOG_FILE << std::endl;
        return;
    }
    errorLog << getCurrentTimestamp() << " [ERROR] " << errorMessage << std::endl;
    errorLog.close();
}

/**
 * @brief Clears all log entries from the database.
 */
void Logger::clearLogs(
#ifdef USE_SOURCE_INFO
    const bool clearSources
#endif
)
{
    writer.clearLogs();

#ifdef USE_SOURCE_INFO
    if(clearSources)
    {
        writer.clearSources();

        sourceId = SOURCE_NOT_FOUND;
        sourceInfo.reset();
    }
#endif
}

/**
 * @brief Retrieves statistics about the logger.
 * @return The statistics about the logger.
 */
Logger::Stats Logger::getStats() const
{
    std::scoped_lock lock(statsMutex);
    return Stats
    {
        totalTasksProcessed,
        totalTasksProcessed > 0 ? totalProcessingTime / totalTasksProcessed : 0,
        maxProcessingTime
    };
}

/**
* @brief Retrieves log entries from the database matching specified filters.
*
* @param filters Vector of Filter objects defining search criteria.
*        Each filter specifies:
*        - Field name (e.g., "level", "timestamp")
*        - Comparison operator ("=", ">", "<=" etc.)
*        - Value to match
*        - Optional type hint (for special handling)
*
* @param limit Maximum number of log entries to return.
*        - Use -1 for no limit (default)
*        - Positive values enable pagination
*
* @param offset Number of log entries to skip before returning results.
*        - Use -1 to disable (default)
*        - Requires positive limit to take effect
*
* @return LogEntryList List of log entries ordered by timestamp (descending).
*/
LogEntryList Logger::getLogsByFilters(const std::vector<Filter> & filters,
                                      const int limit,
                                      const int offset)
{
    if(!waitUntilEmpty())
    {
        logError(ERR_MSG_TIMEOUT_TASK_QUEUE);
    }
    std::scoped_lock lock(logMutex, dbMutex);
    return reader.getLogsByFilters(filters, limit, offset);
}

/**
 * @brief Retrieves all log entries.
 * @return A list of all log entries.
 */
LogEntryList Logger::getAllLogs()
{
    return getLogsByFilters({});
}

/**
 * @brief Retrieves log entries with the specified level.
 * @param limit Maximum number of log entries to return.
 * @param offset Number of log entries to skip before returning results.
 * @return A list of log entries with the specified level.
 */
LogEntryList Logger::getLogsByLevel(const LogLevel& level,
                                    const int limit,
                                    const int offset)
{
    Filter filter;
    filter.type = Filter::Type::Level;
    filter.field = filter.typeToField();
    filter.op = "=";
    filter.value = levelToString(level);
    return getLogsByFilters({ filter }, limit, offset);
}

/**
 * @brief Retrieves log entries within a specified timestamp range.
 * @param startTime The start of the timestamp range.
 * @param endTime The end of the timestamp range.
 * @param limit Maximum number of log entries to return.
 * @param offset Number of log entries to skip before returning results.
 * @return A list of log entries within the specified timestamp range.
 */
LogEntryList Logger::getLogsByTimestampRange(const std::string& startTime, const std::string& endTime,
        const int limit,
        const int offset)
{
    Filter filterStart;
    filterStart.type = Filter::Type::TimestampRange;
    filterStart.field = filterStart.typeToField();
    filterStart.op = ">=";
    filterStart.value = startTime;

    Filter filterEnd;
    filterEnd.type = Filter::Type::TimestampRange;
    filterEnd.field = filterEnd.typeToField();
    filterEnd.op = "<=";
    filterEnd.value = endTime;

    return getLogsByFilters({ filterStart, filterEnd }, limit, offset);
}

/**
 * @brief Retrieves log entries from the specified file.
 * @param file The file to filter by.
 * @param limit Maximum number of log entries to return.
 * @param offset Number of log entries to skip before returning results.
 * @return A list of log entries from the specified file.
 */
LogEntryList Logger::getLogsByFile(const std::string& file,
                                   const int limit,
                                   const int offset)
{
    Filter filter;
    filter.type = Filter::Type::File;
    filter.field = filter.typeToField();
    filter.op = "=";
    filter.value = file;
    return getLogsByFilters({ filter }, limit, offset);
}

/**
 * @brief Retrieves log entries created by the specified thread.
 * @param threadId The thread ID to filter by.
 * @param limit Maximum number of log entries to return.
 * @param offset Number of log entries to skip before returning results.
 * @return A list of log entries created by the specified thread.
 */
LogEntryList Logger::getLogsByThreadId(const std::string& threadId,
                                       const int limit,
                                       const int offset)
{
    Filter filter;
    filter.type = Filter::Type::ThreadId;
    filter.field = filter.typeToField();
    filter.op = "=";
    filter.value = threadId;
    return getLogsByFilters({ filter }, limit, offset);
}

/**
 * @brief Retrieves log entries created in the specified function.
 * @param function The function to filter by.
 * @param limit Maximum number of log entries to return.
 * @param offset Number of log entries to skip before returning results.
 * @return A list of log entries created in the specified function.
 */
LogEntryList Logger::getLogsByFunction(const std::string& function,
                                       const int limit,
                                       const int offset)
{
    Filter filter;
    filter.type = Filter::Type::Function;
    filter.field = filter.typeToField();
    filter.op = "=";
    filter.value = function;
    return getLogsByFilters({ filter }, limit, offset);
}

/**
 * @brief Sets the minimum log level for messages to be logged.
 * @param minLevel The minimum log level.
 */
void Logger::setLogLevel(LogLevel minLevel)
{
    this->minLevel = minLevel;
}

#ifdef USE_SOURCE_INFO
/**
 * @brief Adds a new source to the database.
 * @param name The name of the source.
 * @param uuid The UUID of the source.
 * @return The ID of the newly added source, or SOURCE_NOT_FOUND if the operation failed.
 */
int Logger::addSource(const std::string& name, const std::string& uuid)
{
    std::scoped_lock Lock(dbMutex, sourceMutex);

    if(!sourceInfo.has_value())
    {
        sourceInfo = SourceInfo{};
    }

    int sourceId = writer.addSource(name, uuid);
    if(sourceId == SOURCE_NOT_FOUND)
    {
        logError(ERR_MSG_FAILED_TO_ADD_SOURCE + name);
        //throw std::runtime_error(ERR_MSG_FAILED_TO_ADD_SOURCE + name);
        return SOURCE_NOT_FOUND;
    }

    sourceInfo.value().name = name;
    sourceInfo.value().uuid = uuid;
    sourceInfo.value().sourceId = sourceId;
    this->sourceId = sourceId;

    return sourceId;
}
#endif

#ifdef USE_SOURCE_INFO
/**
 * @brief Retrieves a source by its source ID.
 * @param sourceId The source ID of the source to retrieve.
 * @return An optional containing the source information if found, or std::nullopt otherwise.
 */
std::optional<SourceInfo> Logger::getSourceById(const int sourceId)
{
    return reader.getSourceById(sourceId);
}
#endif

#ifdef USE_SOURCE_INFO
/**
 * @brief Retrieves a source by its UUID.
 * @param uuid The UUID of the source to retrieve.
 * @return An optional containing the source information if found, or std::nullopt otherwise.
 */
std::optional<SourceInfo> Logger::getSourceByUuid(const std::string& uuid)
{
    return reader.getSourceByUuid(uuid);
}
#endif

#ifdef USE_SOURCE_INFO
/**
 * @brief Retrieves a source by its name.
 * @param name The name of the source to retrieve.
 * @return An optional containing the source information if found, or std::nullopt otherwise.
 */
std::optional<SourceInfo> Logger::getSourceByName(const std::string& name)
{
    return reader.getSourceByName(name);
}
#endif

#ifdef USE_SOURCE_INFO
/**
 * @brief Retrieves all sources from the database.
 * @return A vector containing all sources.
 */
std::vector<SourceInfo> Logger::getAllSources()
{
    return reader.getAllSources();
}
#endif

#ifdef USE_SOURCE_INFO
/**
* @brief Retrieves a logs by its source ID.
* @param sourceId The source ID of the source to retrieve.
 * @return A list of log entries associated with the specified source ID.
*/
LogEntryList Logger::getLogsBySourceId(const int& sourceId,
                                       const int limit,
                                       const int offset)
{
    Filter filter;
    filter.type = Filter::Type::SourceId;
    filter.field = filter.typeToField();
    filter.op = "=";
    filter.value = std::to_string(sourceId);
    return getLogsByFilters({ filter }, limit, offset);
}
#endif

#ifdef USE_SOURCE_INFO
/**
* @brief Retrieves a logs by its source UUID.
* @param sourceUuid The UUID of the source to retrieve.
 * @return A list of log entries associated with the specified source UUID.
*/
LogEntryList Logger::getLogsBySourceUuid(const std::string& sourceUuid,
        const int limit,
        const int offset)
{
    const auto sourceInfo = getSourceByUuid(sourceUuid);
    if(!sourceInfo.has_value() || sourceInfo.value().sourceId == SOURCE_NOT_FOUND)
    {
        return LogEntryList{};
    }

    const int sourceId = sourceInfo.value().sourceId;
    return getLogsBySourceId(sourceId, limit, offset);
}
#endif
