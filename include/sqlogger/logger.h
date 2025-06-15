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

#ifndef LOGGER_H
#define LOGGER_H

#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <sstream>
#include <fstream>
#include <iostream>
#include "sqlogger_config.h"
#include "sqlogger/log_entry.h"
#include "sqlogger/log_helper.h"
#include "sqlogger/internal/log_writer.h"
#include "sqlogger/internal/log_reader.h"
#include "sqlogger/internal/log_export.h"
#include "sqlogger/internal/thread_pool.h"
#include "sqlogger/log_config.h"

// Macros for symbol export (for Windows)
#ifdef _WIN32
    #ifdef LOGGER_EXPORTS
        #pragma message("LOGGER_EXPORTS Defined")
        #define LOGGER_API __declspec(dllexport)
    #else
        #pragma message("LOGGER_EXPORTS Not Defined")
        #define LOGGER_API __declspec(dllimport)
    #endif
#else
    #define LOGGER_API
#endif

using namespace LogHelper;

// Macros for simplified logging
#define SQLOG_TRACE(logger)   LogMessage(logger, LogLevel::Trace, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define SQLOG_DEBUG(logger)   LogMessage(logger, LogLevel::Debug, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define SQLOG_INFO(logger)    LogMessage(logger, LogLevel::Info, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define SQLOG_WARNING(logger) LogMessage(logger, LogLevel::Warning, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define SQLOG_ERROR(logger)   LogMessage(logger, LogLevel::Error, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define SQLOG_FATAL(logger)   LogMessage(logger, LogLevel::Fatal, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))

// Log internal error macros
#define LOG_INTERNAL_ERROR(message) logError(message, __func__, __FILE__, __LINE__)

class LogManager; // Forward declaration

/**
 * @class Logger
 * @brief Main logger class for handling log entries.
 */
class LOGGER_API SQLogger
{
    private:

        /**
        * @brief Constructs a Logger object.
        * @param database The database interface to use for logging.
        * @param config LogConfig::Config struct that will be applied.
        */
        SQLogger(std::unique_ptr<IDatabase> database, const LogConfig::Config& config
#ifdef SQLG_USE_SOURCE_INFO
                 , std::optional<SourceInfo> sourceInfo = std::nullopt
#endif
                );

        friend class LogManager;

    public:

        SQLogger(const SQLogger&) = delete;
        SQLogger& operator=(const SQLogger&) = delete;

        SQLogger(SQLogger&&) noexcept = default;
        SQLogger& operator=(SQLogger&&) noexcept = default;

        /**
         * @brief Destructor for Logger. Stops all threads and releases resources.
         */
        ~SQLogger();

        /**
         * @enum Sort
         * @brief Enumeration representing the sorting order for log entries.
         */
        enum class Sort
        {
            AsIs = -1, /**< No sorting. */
            Ascending, /**< Sort in ascending order. */
            Descending /**< Sort in descending order. */
        }; // TODO:

        /**
         * @struct Stats
         * @brief Structure representing statistics about the logger.
         */
        struct Stats
        {
            uint64_t totalLogged = 0;
            uint64_t totalFailed = 0;
            uint64_t maxBatchSize = 0;
            uint64_t minBatchSize = 0;
            double avgBatchSize = 0.0;
            uint64_t maxProcessTimeMs = 0;
            uint64_t totalProcessTimeMs = 0;
            uint32_t flushCount = 0;

            double avgProcessTime() const
            {
                return (totalLogged > 0)
                       ? static_cast<double>(totalProcessTimeMs) / totalLogged
                       : 0.0;
            };
        };

        /**
        * @brief Retrieves statistics about the logger.
        * @return The statistics about the logger.
        * @see Stats
        */
        Stats getStats() const;

        /**
        * @brief Resets all logging statistics to zero.
        * Atomically clears all accumulated performance metrics including:
        * - Total logged/failed entries
        * - Batch processing statistics
        * - Timing measurements
        * @see getStats()
        */
        void resetStats();

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
        static std::string getFormattedStats(const Stats);

        /**
        * @brief Generates a human-readable string representation of the current logging statistics.
        * Formats all collected performance metrics into a multi-section text report with:
        * - Entry counts (total and failed)
        * - Batch processing statistics
        * - Timing measurements
        * @return std::string Formatted report with newline-separated sections:
        * @see Stats
        * @see getStats()
        */
        std::string getFormattedStats() const;

        /**
        * @brief Logs a message with the specified level and details.
        * @param level The severity level of the log message.
        * @param message The log message.
        */
        void log(const LogLevel level, const std::string& message);

        /**
         * @brief Clears all log entries from the database.
         */
        void clearLogs(
#ifdef SQLG_USE_SOURCE_INFO
            const bool clearSources = false
#endif
        );

        /**
        * @brief Retrieves log entries from the database matching specified filters.
        *
        * @param filters Vector of Filter objects defining search criteria.
        *        Each filter specifies:
        *        - Field name (e.g., "level", "timestamp")
        *        - Comparison operator ("=", ">", "<=" etc.)
        *        - Value to match
        *        - Optional type hint (for special handling)
        * @param limit Maximum number of log entries to return.
        *        - Use -1 for no limit (default)
        *        - Positive values enable pagination
        * @param offset Number of log entries to skip before returning results.
        *        - Use -1 to disable (default)
        *        - Requires positive limit to take effect
        * @return LogEntryList List of log entries ordered by timestamp (descending).
        */
        LogEntryList getLogsByFilters(const std::vector<Filter> & filters,
                                      const int limit = -1,
                                      const int offset = -1);

        /**
         * @brief Retrieves all log entries.
         * @param limit Maximum number of log entries to return.
         * @param offset Number of log entries to skip before returning results.
         * @return A list of all log entries.
         */
        LogEntryList getAllLogs(const int limit = -1,
                                const int offset = -1);

        /**
         * @brief Retrieves log entries with the specified level.
         * @param level The severity level to filter by.
         * @param limit Maximum number of log entries to return.
         * @param offset Number of log entries to skip before returning results.
         * @return A list of log entries with the specified level.
         */
        LogEntryList getLogsByLevel(const LogLevel& level,
                                    const int limit = -1,
                                    const int offset = -1);

        /**
         * @brief Retrieves log entries within a specified timestamp range.
         * @param startTime The start of the timestamp range.
         * @param endTime The end of the timestamp range.
         * @param limit Maximum number of log entries to return.
         * @param offset Number of log entries to skip before returning results.
         * @return A list of log entries within the specified timestamp range.
         */
        LogEntryList getLogsByTimestampRange(const std::string& startTime, const std::string& endTime,
                                             const int limit = -1,
                                             const int offset = -1);

        /**
         * @brief Retrieves log entries from the specified file.
         * @param file The file to filter by.
         * @param limit Maximum number of log entries to return.
         * @param offset Number of log entries to skip before returning results.
         * @return A list of log entries from the specified file.
         */
        LogEntryList getLogsByFile(const std::string& file,
                                   const int limit = -1,
                                   const int offset = -1);

        /**
         * @brief Retrieves log entries created by the specified thread.
         * @param threadId The thread ID to filter by.
         * @param limit Maximum number of log entries to return.
         * @param offset Number of log entries to skip before returning results.
         * @return A list of log entries created by the specified thread.
         */
        LogEntryList getLogsByThreadId(const std::string& threadId,
                                       const int limit = -1,
                                       const int offset = -1);

        /**
         * @brief Retrieves log entries created in the specified function.
         * @param function The function to filter by.
         * @param limit Maximum number of log entries to return.
         * @param offset Number of log entries to skip before returning results.
         * @return A list of log entries created in the specified function.
         */
        LogEntryList getLogsByFunction(const std::string& function,
                                       const int limit = -1,
                                       const int offset = -1);

        /**
         * @brief Sets the minimum log level for messages to be logged.
         * @param minLevel The minimum log level.
         */
        void setLogLevel(LogLevel minLevel);

        /**
         * @brief Waits until the task queue is empty.
         * @param timeout The maximum time to wait.
         * @return True if the queue is empty within the timeout, false otherwise.
         */
        bool waitUntilEmpty(const std::chrono::milliseconds& timeout = std::chrono::milliseconds(1000));

        /**
        * @brief Exports log entries to a specified format file.
        * @param filePath The path to the output file.
        * @param format Format of the output file.
        * @param entryList The list of log entries to export.
        * @param delimiter The delimiter to use between fields.
        * @param name Whether to include field names in the output.
        */
        static void exportTo(const std::string& filePath, const LogExport::Format& format, const LogEntryList& entryList, const std::string& delimiter = ENTRY_DELIMITER, bool name = true);

        void setErrorLogPath(const std::string& errLogFile);

        /**
        * @brief Logs an error message to the error log file.
        * @param errorMessage The error message to log.
        * @param function The function where the log message was created.
        * @param file The file where the log message was created.
        * @param line The line number where the log message was created.
        */
        void logError(const std::string& errorMessage,
                      const std::string& function,
                      const std::string& file,
                      const int line);

        /**
         * @brief Forces immediate processing (writes to the database) of all batched log entries, if useBatch = true.
         * @see Config
         * @see processBatch()
         * @see syncMode
         */
        void flush();

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
        void setBatchSize(const int size);

        /**
         * @brief Gets the current logger configuration.
         * @return LogConfig::Config A copy of the current configuration object.
         * @note This method is thread-safe due to internal mutex protection.
         * @see LogConfig::Config
         */
        LogConfig::Config getConfig() const;

        /**
        * @brief Checks if batch logging mode is currently enabled.
        * @return bool Current batch mode status (matches useBatch config value)
        * @see setBatchSize()
        * @see flushBatch()
        * @see useBatch
        * @see setBatchSize
        */
        bool isBatchEnabled() const;

        /**
         * @brief Gets the current batch size used for buffered logging operations.
         * @return int The maximum number of log entries that will be buffered
         * before being written to the database. Returns 0 if batching is disabled.
         * @see setBatchSize()
         */
        int getBatchSize() const;

        /**
         * @brief Gets the type of database currently used by the logger.
         * @return DataBaseType The database type (SQLite, MySQL, PostgreSQL, etc.).
         * @see DataBaseType
         */
        DataBaseType getDataBaseType();

        /**
         * @brief Gets the minimum log level that will be processed by the logger.
         * @return LogLevel The current minimum log level.
         * @see setLogLevel()
         */
        LogLevel getMinLogLevel() const;

        /**
         * @brief Gets the number of worker threads used for asynchronous logging.
         * @return size_t Number of active worker threads processing log entries.
         * Returns 0 if in synchronous mode (isSyncMode() == true).
         * @see isSyncMode()
         * @see ThreadPool
         */
        size_t getNumThreads() const;

        /**
        * @brief Checks whether logging operations execute synchronously in calling thread.
        * When true, log operations block until fully completed. When false,
        * logs are queued and processed asynchronously in background threads.
        * @return bool True if logging executes synchronously in calling thread,
        * false if using asynchronous background processing.
        */
        bool isSyncMode() const;

        /**
         * @brief Checks whether logger stores only filenames instead of full paths.
         * When enabled, only the filename portion of source file paths is stored,
         * excluding directory information.
         * @return bool True if only filenames are stored, false if full paths are kept
         * @see LogConfig::Config::onlyFileNames
         */
        bool isOnlyFileNames() const;

        /**
         * @brief Gets the name identifier of this logger instance.
         * The name helps distinguish between different logger instances in systems
         * with multiple loggers.
         * @return std::string Current logger name.
         * @see LogConfig::Config::name
         */
        std::string getName() const;

#ifdef SQLG_USE_SOURCE_INFO
        /**
        * @brief Adds a new source to the database.
        * @param name The name of the source.
        * @param uuid The UUID of the source.
        * @return The ID of the newly added source, or SOURCE_NOT_FOUND if the operation failed.
        */
        int addSource(const std::string& name, const std::string& uuid);

        /**
         * @brief Retrieves a source by its source ID.
         * @param sourceId The source ID of the source to retrieve.
         * @return An optional containing the source information if found, or std::nullopt otherwise.
         */
        std::optional<SourceInfo> getSourceById(const int sourceId);

        /**
         * @brief Retrieves a source by its UUID.
         * @param uuid The UUID of the source to retrieve.
         * @return An optional containing the source information if found, or std::nullopt otherwise.
         */
        std::optional<SourceInfo> getSourceByUuid(const std::string& uuid);

        /**
         * @brief Retrieves a source by its name.
         * @param name The name of the source to retrieve.
         * @return An optional containing the source information if found, or std::nullopt otherwise.
         */
        std::optional<SourceInfo> getSourceByName(const std::string& name);

        /**
        * @brief Retrieves all sources from the database.
        * @return A vector containing all sources.
        */
        std::vector<SourceInfo> getAllSources();

        /**
        * @brief Retrieves a logs by its source ID.
        * @param sourceId The source ID of the source to retrieve.
         * @return A list of log entries associated with the specified source ID.
        */
        LogEntryList getLogsBySourceId(const int& sourceId,
                                       const int limit = -1,
                                       const int offset = -1);

        /**
        * @brief Retrieves a logs by its source UUID.
        * @param sourceUuid The UUID of the source to retrieve.
         * @return A list of log entries associated with the specified source UUID.
        */
        LogEntryList getLogsBySourceUuid(const std::string& sourceUuid,
                                         const int limit = -1,
                                         const int offset = -1);
#endif

    private:
        /**
         * @struct LogTask
         * @brief Structure representing a log task.
         */
        struct LogTask
        {
            LogLevel level; /**< The severity level of the log task. */
            std::string message; /**< The log message. */
            std::string function; /**< The function where the log task was created. */
            std::string file; /**< The file where the log task was created. */
            int line; /**< The line number where the log task was created. */
            std::string threadId; /**< The ID of the thread that created the log task. */
            std::chrono::system_clock::time_point timestamp; /**< The timestamp of the log task. */
#ifdef SQLG_USE_SOURCE_INFO
            int sourceId;  /**< The source ID. */
#endif
        };

        friend class LogMessage;

        /**
        * @brief Logs a message with the specified level and details.
        * @param level The severity level of the log message.
        * @param message The log message.
        * @param function The function where the log message was created.
        * @param file The file where the log message was created.
        * @param line The line number where the log message was created.
        * @param threadId The ID of the thread that created the log message.
        */
        void logAdd(const LogLevel level, const std::string& message, const std::string& function, const std::string& file, int line, const std::string& threadId);

        /**
         * @brief Processes a single log task.
         * @param task The log task to process.
         */
        void processTask(const LogTask& task);

        /**
         * @brief Processes a batch of log tasks.
         * @param batch The batch of log tasks to process.
         */
        void processBatch(const std::vector<LogTask> & batch);

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
        void flushBatch();

        /**
        * @brief Converts an internal LogTask structure to a persistent LogEntry.
        * @param task The source LogTask containing raw logging information.
        * @return LogEntry Log entry ready for storage.
        * @note Automatically generates timestamp and log level in string representation.
        * @warning The returned entry's ID field will be 0 until stored in database.
        * @warning Returned LogEntry not contain SourceInfo information.
        */
        LogEntry convertTaskToEntry(const LogTask& task) const;

        /**
         * @brief Updates statistics for a single log entry processing operation.
         * This method updates internal performance metrics when a single log entry
         * has been processed (written to database or exported).
         * @param processTimeMs The time taken to process the log entry in milliseconds.
         * @param success Whether the operation was successful (default: true).
         * If false, increments the failed entries counter.
         */
        void updateSingleEntryStats(const uint64_t processTimeMs, const bool success = true);

        /**
         * @brief Updates statistics for batch log entries processing.
         * Updates aggregated performance metrics when a batch of log entries
         * has been processed. Handles both successful and failed batch operations.
         * @param batchSize Number of log entries in the processed batch.
         * @param processTimeMs Total time taken to process the entire batch in milliseconds.
         * @param success Whether the batch operation succeeded (default: true).
         * If false, all entries in batch are counted as failed.
         */
        void updateBatchStats(const size_t batchSize, const uint64_t processTimeMs, const bool success = true);

        /**
        * @brief Shuts down the logger and stops all worker threads.
        */
        void shutdown();

        std::mutex logMutex; /**< Mutex for log access synchronization. */

        std::mutex dbMutex; /**< Mutex for database access synchronization. */
        std::unique_ptr<IDatabase> database; /**< The database interface used for logging. */

        mutable std::mutex configMutex; /**< Mutex for protecting configuration access. Marked mutable to allow locking in const methods.*/
        LogConfig::Config config; /**< Current configuration settings for the logger. */

        LogWriter writer; /**< The log writer used for writing log entries. */
        LogReader reader; /**< The log reader used for reading log entries. */

        ThreadPool threadPool; /**< The thread pool for processing log tasks. */

        std::atomic<bool> running; /**< Flag indicating whether the logger is running. */

        mutable std::mutex statsMutex; /**< Mutex for statistics synchronization. */
        Stats currentStats; /**< Current logger statistics. */

        std::recursive_mutex batchMutex; /**< Mutex for batch access synchronization. */
        std::vector<LogTask> batchBuffer; /**< Batch buffer (LogTasks). */

#ifdef SQLG_USE_SOURCE_INFO
        std::atomic<int> sourceId; /**< The source ID. */
        std::optional<SourceInfo> sourceInfo; /**< The source info. */
        mutable std::mutex sourceMutex;  /**< Mutex for source info synchronization. */
#endif
        std::mutex errorLogMutex;
        std::string errorLogFile = ERR_LOG_FILE;

};

/**
 * @class LogMessage
 * @brief Helper class for simplified logging.
 */
class LogMessage
{
    public:
        /**
         * @brief Constructs a LogMessage object.
         * @param logger The logger to use for logging.
         * @param level The severity level of the log message.
         * @param func The function where the log message was created.
         * @param file The file where the log message was created.
         * @param line The line number where the log message was created.
         * @param threadId The ID of the thread that created the log message.
         */
        LogMessage(SQLogger& logger, LogLevel level, const std::string& func, const std::string& file, int line, const std::string& threadId)
            : logger(logger), level(level), func(func), file(file), line(line), threadId(threadId) {}

        LogMessage(const LogMessage&) = delete; /**< Deleted copy constructor. */
        LogMessage& operator=(const LogMessage&) = delete; /**< Deleted copy assignment operator. */

        LogMessage(LogMessage&&) = default; /**< Default move constructor. */
        LogMessage& operator=(LogMessage&&) = default; /**< Default move assignment operator. */

        /**
         * @brief Overloads the << operator for appending to the log message.
         * @tparam T The type of the value to append.
         * @param value The value to append to the log message.
         * @return A reference to the LogMessage object.
         */
        template<typename T>
        LogMessage& operator<<(const T& value)
        {
            stream << value;
            return *this;
        }

        /**
         * @brief Destructor for LogMessage. Logs the message when the object is destroyed.
         */
        ~LogMessage()
        {
            logger.logAdd(level, stream.str(), func, file, line, threadId);
        }

    private:
        SQLogger& logger; /**< The logger used for logging. */
        LogLevel level; /**< The severity level of the log message. */
        std::string func; /**< The function where the log message was created. */
        std::string file; /**< The file where the log message was created. */
        int line; /**< The line number where the log message was created. */
        std::string threadId; /**< The ID of the thread that created the log message. */
        std::ostringstream stream; /**< The stream used to build the log message. */
};

#endif // LOGGER_H