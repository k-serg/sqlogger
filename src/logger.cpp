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
      config(config),
      writer( * this->database, config.databaseTable.value_or(LOG_TABLE_NAME)),
      reader( * this->database, config.databaseTable.value_or(LOG_TABLE_NAME)),
      threadPool(config.numThreads.value_or(LOG_DEFAULT_NUM_THREADS)),
      running(true)
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
    if(config.useBatch.value())
    {
        flushBatch();
    }
    shutdown();
}

/**
 * @brief Shuts down the logger and stops all worker threads.
 */
void Logger::shutdown()
{
    running = false;
    if(!config.syncMode.value())
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
    if(level < config.minLogLevel.value()) return;

    std::string fileName(file);
    if(config.onlyFileNames.value())
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

    LogTask task
    {
        level,
        message,
        function,
        fileName,
        line,
        threadId,
        std::chrono::system_clock::now()
#ifdef USE_SOURCE_INFO
        , sourceId
#endif
    };

    if(config.useBatch.value())
    {
        std::lock_guard<std::recursive_mutex> lock(batchMutex);
        batchBuffer.push_back(task);

        if(batchBuffer.size() >= config.batchSize.value())
        {
            flushBatch();
        }
    }
    else
    {
        if(config.syncMode.value())
        {
            processTask(task);
        }
        else
        {
            threadPool.enqueue([this, task]
            {
                processTask(task);
            });
        }
    }
}

/**
 * @brief Waits until the task queue is empty.
 * @param timeout The maximum time to wait.
 * @return True if the queue is empty within the timeout, false otherwise.
 */
bool Logger::waitUntilEmpty(const std::chrono::milliseconds& timeout)
{
    if(config.syncMode.value())
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
    auto startTime = std::chrono::high_resolution_clock::now();
    bool success = false;

    try
    {
        std::scoped_lock lock(dbMutex, statsMutex);
        std::string levelStr = levelToString(task.level);
        std::string timestamp = getCurrentTimestamp();

        LogEntry entry = convertTaskToEntry(task);

        if(!writer.writeLog(entry))
        {
            success = false;
            logError(ERR_MSG_FAILED_QUERY);
        }

        success = true;
    }
    catch(const std::exception& e)
    {
        success = false;
        logError(ERR_MSG_FAILED_TASK + std::string(e.what()));
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto taskTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    updateSingleEntryStats(taskTime, success);
}

/**
 * @brief Processes a batch of log tasks.
 * @param batch The batch of log tasks to process.
 */
void Logger::processBatch(const std::vector<LogTask> & batch)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    bool success = false;

    try
    {
        std::scoped_lock lock(dbMutex, statsMutex);

        // Convert tasks to entries
        LogEntryList entries;
        entries.reserve(batch.size());
        for(const auto & task : batch)
        {
            entries.push_back(convertTaskToEntry(task));
        }

        if(!writer.writeLogBatch(entries))
        {
            success = false;
            logError(ERR_MSG_FAILED_BATCH_QUERY);
        }

        success = true;
    }
    catch(const std::exception& e)
    {
        success = false;
        logError(ERR_MSG_FAILED_BATCH_TASK + std::string(e.what()));
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto batchTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    updateBatchStats(batch.size(), batchTime, success);
}

/**
 * @brief Forces immediate processing (writes to the database) of all batched log entries, if useBatch = true.
 * @see Config
 * @see processBatch()
 * @see syncMode
 */
void Logger::flush()
{
    if(config.useBatch.value())
    {
        flushBatch();
    }
}

/**
 * @brief Gets the current logger configuration.
 * @return LogConfig::Config A copy of the current configuration object.
 * @note This method is thread-safe due to internal mutex protection.
 * @see LogConfig::Config
 */
LogConfig::Config Logger::getConfig() const
{
    std::lock_guard<std::mutex> lock(configMutex);
    return config;
}

/**
 * @brief Checks if batch logging mode is currently enabled.
 * @return bool Current batch mode status (matches useBatch config value)
 * @see setBatchSize()
 * @see flushBatch()
 * @see useBatch
 * @see setBatchSize
 */
bool Logger::isBatchEnabled() const
{
    return config.useBatch.value();
}

/**
 * @brief Gets the current batch size used for buffered logging operations.
 * @return int The maximum number of log entries that will be buffered
 * before being written to the database. Returns 0 if batching is disabled.
 * @see setBatchSize()
 */
int Logger::getBatchSize() const
{
    return isBatchEnabled() ? config.batchSize.value() : 0;
}

/**
 * @brief Sets the maximum batch size for buffered logging.
 * Updates the batch size configuration and ensures thread-safe operation:
 * 1. If reducing batch size, flushes current buffer if it exceeds new size
 * 2. If disabling batching (size=0), performs immediate flush
 * @param size New maximum batch size (0 to disable batching)
 * @throws std::invalid_argument If size is negative or bigger than DB_MAX_BATCH_
 * @see flush()
 * @see isBatchEnabled()
 * @see DataBaseHelper::getMaxBatchSize()
 * @see DB_MAX_BATCH_SQLITE, DB_MAX_BATCH_MYSQL, DB_MAX_BATCH_POSTGRESQL.
 */
void Logger::setBatchSize(const int size)
{
    const int maxBatchSize = DataBaseHelper::getMaxBatchSize(database->getDatabaseType());

    if(size > maxBatchSize || maxBatchSize == DB_BATCH_NOT_SUPPORTED)
    {
        throw std::invalid_argument("Batch size for selected database can't be negative or bigger than: " + maxBatchSize);
    }

    if(size < 0)
    {
        throw std::invalid_argument("Batch size can't be negative");
    }

    std::lock_guard<std::recursive_mutex> lock(batchMutex);

    // Flush if: 1) Disabling batching, or 2) Shrinking below current buffer size
    if((size == 0 && config.useBatch.value()) || (config.useBatch.value() && batchBuffer.size() > size))
    {
        flushBatch();
    }

    config.batchSize.value() = size;
    config.useBatch.value() = (size > 0);
}

/**
 * @brief Forces immediate processing (writes to the database) of all batched log entries, if useBatch = true.
 * This method:
 * 1. Atomically moves all entries from the batch buffer to a local vector
 * 2. Processes them either:
 * - Synchronously (in current thread) if syncMode=true
 * - Asynchronously (via thread pool) if syncMode=false
 * @see Config
 * @see processBatch()
 * @see syncMode
 */
void Logger::flushBatch()
{
    if(batchBuffer.empty()) return;

    std::vector<LogTask> currentBatch;
    {
        std::lock_guard<std::recursive_mutex> lock(batchMutex);
        currentBatch.swap(batchBuffer);
    }

    if(config.syncMode.value())
    {
        processBatch(currentBatch);
    }
    else
    {
        threadPool.enqueue([this, currentBatch]
        {
            processBatch(currentBatch);
        });
    }
}

/**
 * @brief Converts an internal LogTask structure to a persistent LogEntry.
 * @param task The source LogTask containing raw logging information.
 * @return LogEntry Log entry ready for storage.
 * @note Automatically generates timestamp and log level in string representation.
 * @warning The returned entry's ID field will be 0 until stored in database.
 * @warning Returned LogEntry not contain SourceInfo information.
 */
LogEntry Logger::convertTaskToEntry(const LogTask& task) const
{
    std::string fileName(task.file);
    if(config.onlyFileNames.value())
    {
        fileName = std::filesystem::path(fileName).filename().string();
    }

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
        , "" // uuid (empty)
        , "" // sourceName (empty)
#endif
    };

    return entry;
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
 * @brief Updates statistics for a single log entry processing operation.
 * This method updates internal performance metrics when a single log entry
 * has been processed (written to database or exported).
 * @param processTimeMs The time taken to process the log entry in milliseconds.
 * @param success Whether the operation was successful (default: true).
 * If false, increments the failed entries counter.
 */
void Logger::updateSingleEntryStats(const uint64_t processTimeMs,
                                    const bool success)
{
    std::lock_guard<std::mutex> lock(statsMutex);

    currentStats.totalLogged++;
    if(!success) currentStats.totalFailed++;

    if(processTimeMs > currentStats.maxProcessTimeMs)
    {
        currentStats.maxProcessTimeMs = processTimeMs;
    }
    currentStats.totalProcessTimeMs += processTimeMs;
}

/**
 * @brief Updates statistics for batch log entries processing.
 * Updates aggregated performance metrics when a batch of log entries
 * has been processed. Handles both successful and failed batch operations.
 * @param batchSize Number of log entries in the processed batch.
 * @param processTimeMs Total time taken to process the entire batch in milliseconds.
 * @param success Whether the batch operation succeeded (default: true).
 * If false, all entries in batch are counted as failed.
 */
void Logger::updateBatchStats(const size_t batchSize,
                              const uint64_t processTimeMs,
                              const bool success)
{
    std::lock_guard<std::mutex> lock(statsMutex);

    currentStats.totalLogged += batchSize;
    if(!success) currentStats.totalFailed += batchSize;

    currentStats.maxBatchSize = std::max(currentStats.maxBatchSize, batchSize);

    currentStats.minBatchSize = currentStats.flushCount == 0 ? batchSize : std::min(currentStats.minBatchSize, batchSize);

    currentStats.avgBatchSize = (currentStats.avgBatchSize* currentStats.flushCount + batchSize) /
                                (currentStats.flushCount + 1);

    currentStats.maxProcessTimeMs = std::max(currentStats.maxProcessTimeMs, processTimeMs);

    currentStats.totalProcessTimeMs += processTimeMs;
    currentStats.flushCount++;
}

/**
 * @brief Retrieves statistics about the logger.
 * @return The statistics about the logger.
 */
Logger::Stats Logger::getStats() const
{
    std::lock_guard<std::mutex> lock(statsMutex);
    return currentStats;
}

/**
 * @brief Resets all logging statistics to zero.
 * Atomically clears all accumulated performance metrics including:
 * - Total logged/failed entries
 * - Batch processing statistics
 * - Timing measurements
 * @see getStats()
 */
void Logger::resetStats()
{
    std::lock_guard<std::mutex> lock(statsMutex);
    currentStats = {};
}

/**
 * @brief Generates a human-readable string representation of logging statistics (static method).
 * Formats all collected performance metrics into a multi-section text report with:
 * - Entry counts (total and failed)
 * - Batch processing statistics
 * - Timing measurements
 * @param stats The Stats structure containing metrics to format
 * @return std::string Formatted report with newline-separated sections:
 * @see Stats
 * @see getStats()
 */
std::string Logger::getFormattedStats(const Stats stats)
{
    std::ostringstream ss;

    ss << "Logging statistics:" << std::endl
       << "[Entries]" << std::endl
       << "Total entries: " << stats.totalLogged << "" << std::endl
       << "Failed entries: " << stats.totalFailed << "" << std::endl
       << "[Batch statistics]" << std::endl
       << "Max size: " << stats.maxBatchSize << "" << std::endl
       << "Min size: " << stats.minBatchSize << "" << std::endl
       << "Avg size: " << std::fixed << std::setprecision(2) << stats.avgBatchSize << "" << std::endl
       << "Flush operations: " << stats.flushCount << "" << std::endl
       << "[Performance]" << std::endl
       << "Max process time: " << stats.maxProcessTimeMs << " ms" << std::endl
       << "Avg process time: " << stats.avgProcessTime() << " ms" << std::endl;
    return ss.str();
}

/**
* @brief Generates a human-readable string representation of the current logging statistics.
* Formats all collected performance metrics into a multi-section text report with:
* - Entry counts (total and failed)
* - Batch processing statistics
* - Timing measurements
* @param stats The Stats structure containing metrics to format
* @return std::string Formatted report with newline-separated sections:
* @see Stats
* @see getStats()
*/
std::string Logger::getFormattedStats() const
{
    std::lock_guard<std::mutex> lock(statsMutex);
    return Logger::getFormattedStats(currentStats);
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
    config.minLogLevel.value() = minLevel;
}

/**
 * @brief Gets the minimum log level that will be processed by the logger.
 * @return LogLevel The current minimum log level.
 * @see setLogLevel()
 */
LogLevel Logger::getMinLogLevel() const
{
    return config.minLogLevel.value();
}

/**
 * @brief Gets the number of worker threads used for asynchronous logging.
 * @return size_t Number of active worker threads processing log entries.
 * Returns 0 if in synchronous mode (isSyncMode() == true).
 * @see isSyncMode()
 * @see ThreadPool
 */
size_t Logger::getNumThreads() const
{
    return isSyncMode() ? 0 : config.numThreads.value();
}

/**
 * @brief Gets the type of database currently used by the logger.
 * @return DataBaseType The database type (SQLite, MySQL, PostgreSQL, etc.).
 * @see DataBaseType
 */
DataBaseType Logger::getDataBaseType()
{
    std::scoped_lock lock(dbMutex);
    return database->getDatabaseType();
}

/**
 * @brief Checks whether logging operations execute synchronously in calling thread.
 * When true, log operations block until fully completed. When false,
 * logs are queued and processed asynchronously in background threads.
 * @return bool True if logging executes synchronously in calling thread,
 * false if using asynchronous background processing.
 */
bool Logger::isSyncMode() const
{
    return config.syncMode.value();
}

/**
 * @brief Checks whether logger stores only filenames instead of full paths.
 * When enabled, only the filename portion of source file paths is stored,
 * excluding directory information.
 * @return bool True if only filenames are stored, false if full paths are kept
 * @see LogConfig::Config::onlyFileNames
 */
bool Logger::isOnlyFileNames() const
{
    return config.onlyFileNames.value();
}

/**
 * @brief Gets the name identifier of this logger instance.
 * The name helps distinguish between different logger instances in systems
 * with multiple loggers.
 * @return std::string Current logger name.
 * @see LogConfig::Config::name
 */
std::string Logger::getName() const
{
    return config.name.value();
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
