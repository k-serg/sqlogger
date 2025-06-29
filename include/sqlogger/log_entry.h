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
#include <algorithm>
#include "sqlogger/internal/log_strings.h"

#ifdef SQLG_USE_SOURCE_INFO
    #pragma message("SOURCE INFO support enabled.")
    #include <3rdparty/stduuid/include/uuid.h>
#endif

#ifdef SQLG_USE_SOURCE_INFO
    #define SOURCE_NOT_FOUND -1
#endif

// Define constants for table field names
#define LOG_DATABASE_NAME "logs_db"
#define LOG_TABLE_NAME "logs"
#define FIELD_LOG_ID "id"
#define FIELD_LOG_TIMESTAMP "timestamp"
#define FIELD_LOG_LEVEL "level"
#define FIELD_LOG_MESSAGE "message"
#define FIELD_LOG_FUNCTION "func"
#define FIELD_LOG_FILE "file"
#define FIELD_LOG_LINE "line"
#define FIELD_LOG_THREAD_ID "thread_id"
#ifdef SQLG_USE_SOURCE_INFO
    #define FIELD_LOG_SOURCES_ID "source_id"
#endif

#ifdef SQLG_USE_SOURCE_INFO
    #define SOURCES_TABLE_NAME "sources"
    #define FIELD_SOURCES_ID "id"
    #define FIELD_SOURCES_SOURCE_ID "source_id"
    #define FIELD_SOURCES_UUID "uuid"
    #define FIELD_SOURCES_NAME "name"
#endif

#ifdef SQLG_USE_SOURCE_INFO
    #define SOURCE_DEFAULT_NAME "default_source"
#endif

// Define constants for table field file export names
#define EXP_LOG_ENTRIES "LogEntries"
#define EXP_LOG_ENTRY "LogEntry"
#define EXP_FIELD_ID "ID"
#define EXP_FIELD_TIMESTAMP "Timestamp"
#define EXP_FIELD_LEVEL "Level"
#define EXP_FIELD_MESSAGE "Message"
#define EXP_FIELD_FUNCTION "Function"
#define EXP_FIELD_FILE "File"
#define EXP_FIELD_LINE "Line"
#define EXP_FIELD_THREAD_ID "ThreadID"
#ifdef SQLG_USE_SOURCE_INFO
    #define EXP_FIELD_SOURCE "Source"
    #define EXP_FIELD_SOURCE_ID "SourceID"
    #define EXP_FIELD_SOURCE_UUID "SourceUUID"
    #define EXP_FIELD_SOURCE_NAME "SourceName"
#endif

#define EXP_FILTER_FIELD "Field"
#define EXP_FILTER_OP "Op"
#define EXP_FILTER_VALUE "Value"

#define LOG_LEVEL_UNKNOWN "UNKNOWN"
#define LOG_LEVEL_TRACE "TRACE"
#define LOG_LEVEL_DEBUG "DEBUG"
#define LOG_LEVEL_INFO "INFO"
#define LOG_LEVEL_WARNING "WARNING"
#define LOG_LEVEL_ERROR "ERROR"
#define LOG_LEVEL_FATAL "FATAL"

#define ENTRY_DELIMITER ","
#define TIMESTAMP_FMT "%Y-%m-%d %H:%M:%S"

constexpr const char* ALLOWED_FILTER_OP[] =
{
    "=", ">", "<", ">=", "<=", "!=", "<>",
    "LIKE", "NOT LIKE",
    "IN", "NOT IN",
    "IS NULL", "IS NOT NULL"
};

#ifdef SQLG_USE_SOURCE_INFO
/**
 * @brief Represents information about a logging source.
 *
 * This structure holds details about a source that generates log entries,
 * including its unique identifier (ID), UUID, and name.
 */
struct SourceInfo
{
    int sourceId;        /**< The unique identifier (ID) of the source. */
    std::string uuid;    /**< The universally unique identifier (UUID) of the source. */
    std::string name;    /**< The name of the source. */
};
#endif

/**
 * @enum LogLevel
 * @brief Enumeration representing the severity level of a log entry.
 */
enum class LogLevel
{
    Unknown = -1,
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
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
#ifdef SQLG_USE_SOURCE_INFO
        , SourceId
#endif
    };

    Type type; /**< The type to filter on. */
    std::string field; /**< The field to filter on. */
    std::string op;    /**< The operator to use for filtering. */
    std::string value; /**< The value to compare against. */

    /**
    * @brief Checks if the operator is allowed.
    * @return True if the operator is allowed, false otherwise.
    * @throws std::invalid_argument If operator is empty
    */
    bool isAllowedOp() const
    {
        if(op.empty())
        {
            throw std::invalid_argument(ERR_MSG_FILTER_OP_EMPTY);
        }

        for(const auto & allowedOp : ALLOWED_FILTER_OP)
        {
            if(op == allowedOp)
            {
                return true;
            }
        }
        return false;
    };

    /**
    * @brief Converts the field name (string) to the corresponding Filter::Type enum value.
    * @return Filter::Type The corresponding enum value for the field.
    */
    Type fieldToType() const
    {
        if(field == FIELD_LOG_LEVEL) return Type::Level;
        if(field == FIELD_LOG_FILE) return Type::File;
        if(field == FIELD_LOG_FUNCTION) return Type::Function;
        if(field == FIELD_LOG_THREAD_ID) return Type::ThreadId;
        if(field == FIELD_LOG_TIMESTAMP) return Type::TimestampRange;
#ifdef SQLG_USE_SOURCE_INFO
        if(field == FIELD_LOG_SOURCES_ID) return Type::SourceId;
#endif
        return Type::Unknown;
    };

    /**
    * @brief Static method to convert the field name (string) to the corresponding Filter::Type enum value.
    * @return Filter::Type The corresponding enum value for the field.
    */
    static Type fieldToType(const std::string& field)
    {
        if(field == FIELD_LOG_LEVEL) return Type::Level;
        if(field == FIELD_LOG_FILE) return Type::File;
        if(field == FIELD_LOG_FUNCTION) return Type::Function;
        if(field == FIELD_LOG_THREAD_ID) return Type::ThreadId;
        if(field == FIELD_LOG_TIMESTAMP) return Type::TimestampRange;
#ifdef SQLG_USE_SOURCE_INFO
        if(field == FIELD_LOG_SOURCES_ID) return Type::SourceId;
#endif
        return Type::Unknown;
    };

    /**
    * @brief Converts the Filter::Type enum value to the corresponding field name (string).
    * @return std::string The corresponding field name for the Filter::Type enum value.
    */
    std::string typeToField() const
    {
        switch(type)
        {
            case Type::Level:
                return FIELD_LOG_LEVEL;
            case Type::File:
                return FIELD_LOG_FILE;
            case Type::Function:
                return FIELD_LOG_FUNCTION;
            case Type::ThreadId:
                return FIELD_LOG_THREAD_ID;
            case Type::TimestampRange:
                return FIELD_LOG_TIMESTAMP;
#ifdef SQLG_USE_SOURCE_INFO
            case Type::SourceId:
                return FIELD_LOG_SOURCES_ID;
#endif
            default:
                return "Unknown";
        }
    };

    /**
    * @brief Static method to convert the Filter::Type enum value to the corresponding field name (string).
    * @return std::string The corresponding field name for the Filter::Type enum value.
    */
    static std::string typeToField(const Filter::Type& type)
    {
        switch(type)
        {
            case Type::Level:
                return FIELD_LOG_LEVEL;
            case Type::File:
                return FIELD_LOG_FILE;
            case Type::Function:
                return FIELD_LOG_FUNCTION;
            case Type::ThreadId:
                return FIELD_LOG_THREAD_ID;
            case Type::TimestampRange:
                return FIELD_LOG_TIMESTAMP;
#ifdef SQLG_USE_SOURCE_INFO
            case Type::SourceId:
                return FIELD_LOG_SOURCES_ID;
#endif
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
#ifdef SQLG_USE_SOURCE_INFO
    int sourceId;  /**< The SourceID. */
#endif
    std::string timestamp; /**< The timestamp of the log entry. */
    std::string level; /**< The severity level of the log entry. */
    std::string message; /**< The message of the log entry. */
    std::string function; /**< The function where the log entry was created. */
    std::string file; /**< The file where the log entry was created. */
    int line; /**< The line number where the log entry was created. */
    std::string threadId; /**< The ID of the thread that created the log entry. */
#ifdef SQLG_USE_SOURCE_INFO
    std::string sourceUuid;  /**< The UUID. */
    std::string sourceName;  /**< The name of the source. */
#endif

    /**
     * @brief Converts the log entry to a string.
     * @param delimiter The delimiter to use between fields.
     * @param name Whether to include field names in the output.
     * @return The string representation of the log entry.
     */
    std::string print(const std::string& delimiter = ENTRY_DELIMITER, bool name = true) const
    {
        std::stringstream sout;
        sout << (name ? " " + std::string(EXP_FIELD_TIMESTAMP) + ": " : "") << timestamp
             << delimiter << (name ? " " + std::string(EXP_FIELD_LEVEL) + ": " : "") << level
             << delimiter << (name ? " " + std::string(EXP_FIELD_MESSAGE) + ": " : "") << "\"" << message << "\""
             << delimiter << (name ? " " + std::string(EXP_FIELD_FUNCTION) + ": " : "") << function
             << delimiter << (name ? " " + std::string(EXP_FIELD_FILE) + ": " : "") << file
             << delimiter << (name ? " " + std::string(EXP_FIELD_LINE) + ": " : "") << line
             << delimiter << (name ? " " + std::string(EXP_FIELD_THREAD_ID) + ": " : "") << threadId
#ifdef SQLG_USE_SOURCE_INFO
             << delimiter << (name ? " " + std::string(EXP_FIELD_SOURCE_ID) + ": " : "") << sourceId
             << delimiter << (name ? " " + std::string(EXP_FIELD_SOURCE_UUID) + ": " : "") << sourceUuid
             << delimiter << (name ? " " + std::string(EXP_FIELD_SOURCE_NAME) + ": " : "") << sourceName
#endif
             ;
        return sout.str();
    }

    /**
    * @brief Overloads the `<<` operator for outputting a `LogEntry` object to an output stream.
    *
    * This operator allows a `LogEntry` object to be written to an output stream (e.g., `std::cout`, `std::ofstream`).
    * It uses the `print()` method of the `LogEntry` class to generate a string representation of the object,
    * which is then written to the provided output stream.
    *
    * @param out The output stream where the `LogEntry` object will be written.
    * @param entry The `LogEntry` object to be output.
    * @return A reference to the output stream (`out`) to allow chaining of `<<` operations.
    *
    * @note This operator is defined as a `friend` function to allow access to the private members of `LogEntry`.
    * @see print() For details on how the string representation is generated.
    */
    friend std::ostream& operator<<(std::ostream& out, const LogEntry& entry)
    {
        out << entry.print();
        return out;
    }

    /**
     * @brief Writes the object's string representation to a file.
     *
     * This method writes the string representation of the object to the provided output file stream.
     * The string representation is generated by calling the `print` method, which formats the object's
     * data using the specified delimiter and optionally includes the object's name.
     *
     * @param outFile The output file stream where the data will be written.
     * @param delimiter The delimiter used to separate fields in the string representation (default is ENTRY_DELIMITER).
     * @param name If true, includes the object's name in the string representation (default is true).
     *
     * @note The output file stream (`outFile`) must be opened and writable before calling this method.
     * @see print() For details on how the string representation is generated.
     */
    void printToFile(std::ofstream& outFile, const std::string& delimiter = ENTRY_DELIMITER, bool name = true) const
    {
        outFile << print(delimiter, name) << std::endl;
    }
};

// Alias for a list of log entries
using LogEntryList = std::vector<LogEntry>;

#endif // LOG_ENTRY_H