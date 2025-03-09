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
#include "log_entry.h"
#include "log_writer.h"
#include "log_reader.h"
#include "log_export.h"
#include "thread_pool.h"
#include "log_config.h"

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

// Constants
#define ERR_LOG_FILE "error_log.txt"

// Macros for simplified logging
#define LOG_TRACE(logger)   LogMessage(logger, LogLevel::Trace, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define LOG_DEBUG(logger)   LogMessage(logger, LogLevel::Debug, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define LOG_INFO(logger)    LogMessage(logger, LogLevel::Info, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define LOG_WARNING(logger) LogMessage(logger, LogLevel::Warning, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define LOG_ERROR(logger)   LogMessage(logger, LogLevel::Error, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))
#define LOG_FATAL(logger)   LogMessage(logger, LogLevel::Fatal, __func__, __FILE__, __LINE__, threadIdToString(std::this_thread::get_id()))

/**
 * @class Logger
 * @brief Main logger class for handling log entries.
 */
class LOGGER_API Logger
{
    public:
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
            std::atomic<size_t> totalTasksProcessed; /**< Total number of tasks processed. */
            std::atomic<double> averageProcessingTime; /**< Average processing time per task. */
            std::atomic<double> maxProcessingTime; /**< Maximum processing time for a task. */
        };

        /**
         * @brief Constructs a Logger object.
         * @param database The database interface to use for logging.
         * @param config LogConfig::Config struct that will be applied.
         */
        Logger(std::unique_ptr<IDatabase> database, const LogConfig::Config& config = {});

        /**
         * @brief Destructor for Logger. Stops all threads and releases resources.
         */
        ~Logger();

        /**
         * @brief Shuts down the logger and stops all worker threads.
         */
        void shutdown();

        /**
        * @brief Logs a message with the specified level and details.
        * @param level The severity level of the log message.
        * @param message The log message.
        */
        void log(const LogLevel level, const std::string& message);

        /**
         * @brief Clears all log entries from the database.
         */
        void clearLogs();

        /**
         * @brief Retrieves statistics about the logger.
         * @return The statistics about the logger.
         */
        Stats getStats() const;

        /**
         * @brief Retrieves log entries based on specified filters.
         * @param filters The filters to apply when retrieving logs.
         * @return A list of log entries that match the filters.
         */
        LogEntryList getLogsByFilters(const std::vector<Filter> & filters);

        /**
         * @brief Retrieves all log entries.
         * @return A list of all log entries.
         */
        LogEntryList getAllLogs();

        /**
         * @brief Retrieves log entries with the specified level.
         * @param level The severity level to filter by.
         * @return A list of log entries with the specified level.
         */
        LogEntryList getLogsByLevel(const LogLevel& level);

        /**
         * @brief Retrieves log entries within a specified timestamp range.
         * @param startTime The start of the timestamp range.
         * @param endTime The end of the timestamp range.
         * @return A list of log entries within the specified timestamp range.
         */
        LogEntryList getLogsByTimestampRange(const std::string& startTime, const std::string& endTime);

        /**
         * @brief Retrieves log entries from the specified file.
         * @param file The file to filter by.
         * @return A list of log entries from the specified file.
         */
        LogEntryList getLogsByFile(const std::string& file);

        /**
         * @brief Retrieves log entries created by the specified thread.
         * @param threadId The thread ID to filter by.
         * @return A list of log entries created by the specified thread.
         */
        LogEntryList getLogsByThreadId(const std::string& threadId);

        /**
         * @brief Retrieves log entries created in the specified function.
         * @param function The function to filter by.
         * @return A list of log entries created in the specified function.
         */
        LogEntryList getLogsByFunction(const std::string& function);

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
         * @brief Logs an error message to the error log file.
         * @param errorMessage The error message to log.
         */
        void logError(const std::string& errorMessage);

        std::unique_ptr<IDatabase> database; /**< The database interface used for logging. */
        LogWriter writer; /**< The log writer used for writing log entries. */
        LogReader reader; /**< The log reader used for reading log entries. */
        ThreadPool threadPool; /**< The thread pool for processing log tasks. */

        std::mutex dbMutex; /**< Mutex for database access synchronization. */
        std::mutex logMutex; /**< Mutex for log access synchronization. */

        std::atomic<bool> running; /**< Flag indicating whether the logger is running. */
        size_t totalTasksProcessed; /**< Total number of tasks processed. */
        double totalProcessingTime; /**< Total processing time for all tasks. */
        double maxProcessingTime; /**< Maximum processing time for a single task. */
        mutable std::mutex statsMutex; /**< Mutex for statistics synchronization. */

        LogLevel minLevel; /**< Minimum log level for messages to be logged. */
        bool syncMode; /**< Whether the logger is in synchronous mode. */
        bool onlyFileNames; /**< Log only filenames or full path to the file. */
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
        LogMessage(Logger& logger, LogLevel level, const std::string& func, const std::string& file, int line, const std::string& threadId)
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
        Logger& logger; /**< The logger used for logging. */
        LogLevel level; /**< The severity level of the log message. */
        std::string func; /**< The function where the log message was created. */
        std::string file; /**< The file where the log message was created. */
        int line; /**< The line number where the log message was created. */
        std::string threadId; /**< The ID of the thread that created the log message. */
        std::ostringstream stream; /**< The stream used to build the log message. */
};

#endif // LOGGER_H