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

#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <thread>
#include <chrono>
#include <iomanip>

// Define constants for table field names
#define LOG_TABLE_NAME "logs"
#define FIELD_ID "id"
#define FIELD_TIMESTAMP "timestamp"
#define FIELD_LEVEL "level"
#define FIELD_MESSAGE "message"
#define FIELD_FUNCTION "function"
#define FIELD_FILE "file"
#define FIELD_LINE "line"
#define FIELD_THREAD_ID "thread_id"

// Define constants for table field file export names
#define EXP_FIELD_ID "ID"
#define EXP_FIELD_TIMESTAMP "Timestamp"
#define EXP_FIELD_LEVEL "Level"
#define EXP_FIELD_MESSAGE "Message"
#define EXP_FIELD_FUNCTION "Function"
#define EXP_FIELD_FILE "File"
#define EXP_FIELD_LINE "Line"
#define EXP_FIELD_THREAD_ID "ThreadID"

#define ENTRY_DELIMITER ","
#define TIMESTAMP_FMT "%Y-%m-%d %H:%M:%S"

constexpr char* ALLOWED_FILTER_OP[] = { "=", ">", "<", ">=", "<=", "!=" }; /**< List of allowed operators for Filter structure. */

/**
 * @enum Level
 * @brief Enumeration representing the severity level of a log entry.
 */
enum class Level
{
    Unknown = -1,
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

namespace LogHelper
{
    /**
     * @brief Converts a log level to its string representation.
     * @param level The log level to convert.
     * @return The string representation of the log level.
     */
    static std::string levelToString(Level level)
    {
        switch(level)
        {
            case Level::Trace:
                return "TRACE";
            case Level::Debug:
                return "DEBUG";
            case Level::Info:
                return "INFO";
            case Level::Warning:
                return "WARNING";
            case Level::Error:
                return "ERROR";
            case Level::Fatal:
                return "FATAL";
            default:
                return "UNKNOWN";
        }
    };

    /**
     * @brief Converts a string to a log level.
     * @param levelStr The string representation of the log level.
     * @return The corresponding log level.
     */
    static Level stringToLevel(const std::string& levelStr)
    {
        if(levelStr == "TRACE") return Level::Trace;
        if(levelStr == "DEBUG") return Level::Debug;
        if(levelStr == "INFO") return Level::Info;
        if(levelStr == "WARNING") return Level::Warning;
        if(levelStr == "ERROR") return Level::Error;
        if(levelStr == "FATAL") return Level::Fatal;
        return Level::Unknown;
    };

    /**
     * @brief Gets the current timestamp as a string.
     * @param timeFormat The format of the timestamp.
     * @return The current timestamp as a string.
     */
    static std::string getCurrentTimestamp(const char* timeFormat = TIMESTAMP_FMT)
    {
        auto now = std::time(nullptr);
        auto tm = * std::localtime( & now);
        std::ostringstream oss;
        oss << std::put_time( & tm, timeFormat);
        return oss.str();
    };

    /**
     * @brief Converts a thread ID to a string.
     * @param id The thread ID to convert.
     * @return The string representation of the thread ID.
     */
    static  std::string threadIdToString(std::thread::id id)
    {
        std::ostringstream oss;
        oss << id;
        return oss.str();
    };
};

/**
 * @struct Filter
 * @brief Structure representing a filter for log entries.
 */
struct Filter
{
    /**
    * @enum Type
    * @brief Type of the filter.
    */
    enum class Type
    {
        Unknown = -1,
        Level,
        File,
        Function,
        ThreadId,
        TimestampRange
    };

    Type type; /**< The type to filter on. */
    std::string field; /**< The field to filter on. */
    std::string op;    /**< The operator to use for filtering. */
    std::string value; /**< The value to compare against. */

    /**
     * @brief Checks if the operator is allowed.
     * @return True if the operator is allowed, false otherwise.
     */
    bool isAllowedOp() const
    {
        for(auto const & alowedOp : ALLOWED_FILTER_OP)
        {
            if(alowedOp == op) return true;
        }
        return false;
    };

    /**
    * @brief Converts the field name (string) to the corresponding Filter::Type enum value.
    * @return Filter::Type The corresponding enum value for the field.
    */
    Type fieldToType() const
    {
        if(field == FIELD_LEVEL) return Type::Level;
        if(field == FIELD_FILE) return Type::File;
        if(field == FIELD_FUNCTION) return Type::Function;
        if(field == FIELD_THREAD_ID) return Type::ThreadId;
        if(field == FIELD_TIMESTAMP) return Type::TimestampRange;
        return Type::Unknown;
    };

    /**
    * @brief Converts the Filter::Type enum value to the corresponding field name (string).
    * @return std::string The corresponding field name for the Filter::Type enum value.
    */
    std::string typeToField()
    {
        switch(type)
        {
            case Type::Level:
                return FIELD_LEVEL;
            case Type::File:
                return FIELD_FILE;
            case Type::Function:
                return FIELD_FUNCTION;
            case Type::ThreadId:
                return FIELD_THREAD_ID;
            case Type::TimestampRange:
                return FIELD_TIMESTAMP;
            default:
                return "Unknown";
        }
    };

};

/**
 * @struct LogEntry
 * @brief Structure representing a log entry.
 */
struct LogEntry
{
    int id; /**< The unique identifier of the log entry. */
    std::string timestamp; /**< The timestamp of the log entry. */
    std::string level; /**< The severity level of the log entry. */
    std::string message; /**< The message of the log entry. */
    std::string function; /**< The function where the log entry was created. */
    std::string file; /**< The file where the log entry was created. */
    int line; /**< The line number where the log entry was created. */
    std::string threadId; /**< The ID of the thread that created the log entry. */

    /**
     * @brief Converts the log entry to a string.
     * @param delimiter The delimiter to use between fields.
     * @param name Whether to include field names in the output.
     * @return The string representation of the log entry.
     */
    std::string print(const std::string& delimiter = ",", bool name = true) const
    {
        std::stringstream sout;
        sout << (name ? "Timestamp: " : "") << timestamp
             << delimiter << (name ? " " + std::string(EXP_FIELD_LEVEL) + ": " : "") << level
             << delimiter << (name ? " " + std::string(EXP_FIELD_MESSAGE) + ": " : "") << "\"" << message << "\""
             << delimiter << (name ? " " + std::string(EXP_FIELD_FUNCTION) + ": " : "") << function
             << delimiter << (name ? " " + std::string(EXP_FIELD_FILE) + ": " : "") << file
             << delimiter << (name ? " " + std::string(EXP_FIELD_LINE) + ": " : "") << line
             << delimiter << (name ? " " + std::string(EXP_FIELD_THREAD_ID) + ": " : "") << threadId;
        return sout.str();
    }

    /**
     * @brief Overloads the << operator for outputting LogEntry to a stream.
     */
    friend std::ostream& operator<<(std::ostream& out, const LogEntry& entry)
    {
        out << entry.print();
        return out;
    }

    /**
     * @brief Outputs the log entry to a file.
     */
    void printToFile(std::ofstream& outFile, const std::string& delimiter = ENTRY_DELIMITER, bool name = true) const
    {
        outFile << print(delimiter, name) << std::endl;
    }
};

// Alias for a list of log entries
using LogEntryList = std::vector<LogEntry>;

#endif // LOG_ENTRY_H