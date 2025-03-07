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
 * @param syncMode Whether to use synchronous mode.
 * @param numThreads The number of threads in the ThreadPool.
 * @param onlyFileName Log only filename, without full path.
 */
Logger::Logger(std::unique_ptr<IDatabase> database, const bool syncMode, const size_t numThreads, const bool onlyFileName)
    : database(std::move(database)),
      writer( * this->database),
      reader( * this->database),
      threadPool(numThreads),
      running(true),
      totalTasksProcessed(0),
      totalProcessingTime(0),
      maxProcessingTime(0),
      minLevel(Level::Info),
      syncMode(syncMode),
      onlyFileName(onlyFileName)
{
    std::scoped_lock lock(dbMutex);
    writer.createTable();
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
void Logger::log(const Level level, const std::string& message)
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
void Logger::logAdd(const Level level, const std::string& message, const std::string& function, const std::string& file, int line, const std::string& threadId)
{
    if(level < minLevel) return;

    std::string fileName(file);
    if (onlyFileName)
    {
        fileName = std::filesystem::path(fileName).filename().string();
    }

    if(syncMode)
    {
        LogTask task{ level, message, function, fileName, line, threadId, std::chrono::system_clock::now() };
        processTask(task);
    }
    else
    {
        threadPool.enqueue([this, level, message, function, fileName, line, threadId]
        {
            LogTask task{ level, message, function, fileName, line, threadId, std::chrono::system_clock::now() };
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

            LogEntry entry{ 0, timestamp, levelStr, task.message, task.function, task.file, task.line, task.threadId };
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
        logError("Error in processTask: " + std::string(e.what()));
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

                LogEntry entry{ 0, timestamp, levelStr, task.message, task.function, task.file, task.line, task.threadId };
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
        std::cerr << "Failed to open error log file: " << ERR_LOG_FILE << std::endl;
        return;
    }
    errorLog << getCurrentTimestamp() << " [ERROR] " << errorMessage << std::endl;
}

/**
 * @brief Clears all log entries from the database.
 */
void Logger::clearLogs()
{
    writer.clearLogs();
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
 * @brief Retrieves log entries based on specified filters.
 * @param filters The filters to apply when retrieving logs.
 * @return A list of log entries that match the filters.
 */
LogEntryList Logger::getLogsByFilters(const std::vector<Filter> & filters)
{
    if(!waitUntilEmpty())
    {
        logError("Timeout while waiting for task queue to empty");
    }
    std::scoped_lock lock(logMutex, dbMutex);
    return reader.getLogsByFilters(filters);
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
 * @param level The severity level to filter by.
 * @return A list of log entries with the specified level.
 */
LogEntryList Logger::getLogsByLevel(const Level& level)
{
    Filter filter;
    filter.type = Filter::Type::Level;
    filter.field = filter.typeToField();
    filter.op = "=";
    filter.value = levelToString(level);
    return getLogsByFilters({ filter });
}

/**
 * @brief Retrieves log entries within a specified timestamp range.
 * @param startTime The start of the timestamp range.
 * @param endTime The end of the timestamp range.
 * @return A list of log entries within the specified timestamp range.
 */
LogEntryList Logger::getLogsByTimestampRange(const std::string& startTime, const std::string& endTime)
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

    return getLogsByFilters({ filterStart, filterEnd });
}

/**
 * @brief Retrieves log entries from the specified file.
 * @param file The file to filter by.
 * @return A list of log entries from the specified file.
 */
LogEntryList Logger::getLogsByFile(const std::string& file)
{
    Filter filter;
    filter.type = Filter::Type::File;
    filter.field = filter.typeToField();
    filter.op = "=";
    filter.value = file;
    return getLogsByFilters({ filter });
}

/**
 * @brief Retrieves log entries created by the specified thread.
 * @param threadId The thread ID to filter by.
 * @return A list of log entries created by the specified thread.
 */
LogEntryList Logger::getLogsByThreadId(const std::string& threadId)
{
    Filter filter;
    filter.type = Filter::Type::ThreadId;
    filter.field = filter.typeToField();
    filter.op = "=";
    filter.value = threadId;
    return getLogsByFilters({ filter });
}

/**
 * @brief Retrieves log entries created in the specified function.
 * @param function The function to filter by.
 * @return A list of log entries created in the specified function.
 */
LogEntryList Logger::getLogsByFunction(const std::string& function)
{
    Filter filter;
    filter.type = Filter::Type::Function;
    filter.field = filter.typeToField();
    filter.op = "=";
    filter.value = function;
    return getLogsByFilters({ filter });
}

/**
 * @brief Sets the minimum log level for messages to be logged.
 * @param minLevel The minimum log level.
 */
void Logger::setLogLevel(Level minLevel)
{
    this->minLevel = minLevel;
}
