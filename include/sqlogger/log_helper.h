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

#ifndef LOG_HELPER_H
#define LOG_HELPER_H

#include "sqlogger/log_entry.h"

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
    std::string join(const std::vector<std::string> & parts, const std::string& delimiter);

    /**
    * @brief Splits a string into a vector of strings using a delimiter
    * @param str The string to split
    * @param delimiter The string to use as delimiter
    * @return std::vector<std::string> The resulting vector of strings
    */
    std::vector<std::string> split(const std::string& str, const std::string& delimiter);
};

/**
 * @namespace LogHelper
 * @brief Provides utility functions for logging operations
 * @details This namespace contains various helper functions that support core logging functionality,
 * including log level conversions, timestamp handling, thread management, and string transformations.
 * @see LogLevel for log level enumeration
 * @see TIMESTAMP_FMT for default timestamp format
 */
namespace LogHelper
{

#ifdef SQLG_USE_SOURCE_INFO
    /**
    * @brief Generates a UUID.
    * @return A string representing the generated UUID.
    */
    std::string generateUUID();
#endif

    /**
    * @brief Checks if std::string represents a numeric value
    * @param value std::string to check
    * @return True if numeric, false otherwise
    */
    bool isNumeric(const std::string& value);

    /**
     * @brief Transform string into upper case. ASCII-only.
     * @param input Source string.
     * @return String transformed into upper case.
    */
    std::string toUpperCase(const std::string& input);

    /**
     * @brief Transform string into lower case. ASCII-only.
     * @param input Source string.
     * @return String transformed into lower case.
    */
    std::string toLowerCase(const std::string& input);

    /**
     * @brief Converts a log level to its string representation.
     * @param level The log level to convert.
     * @return The string representation of the log level.
     */
    std::string levelToString(LogLevel level);

    /**
    * @brief Converts a log level to its integer representation.
    * @param level The log level to convert.
    * @return The integer representation of the log level.
    */
    int levelToInt(LogLevel level);

    /**
     * @brief Converts a string to a log level.
     * @param levelStr The string representation of the log level.
     * @return The corresponding log level.
     */
    LogLevel stringToLevel(const std::string& levelStr, const bool ignoreCase = true);

    /**
     * @brief Converts a int to a log level.
     * @param levelInt The integer representation of the log level.
     * @return The corresponding log level.
     */
    LogLevel intToLevel(const int levelInt);

    /**
    * @brief Converts a time_point to a formatted string representation
    * @param tp The time_point to format
    * @param timeFormat The format string (default: TIMESTAMP_FMT)
    * @return std::string Formatted time string
    * @note Uses the system's local time zone for conversion
    * @see TIMESTAMP_FMT for default format
    */
    std::string formatTime(const std::chrono::system_clock::time_point& tp, const char* timeFormat = TIMESTAMP_FMT);

    /**
    * @brief Parses a formatted time string into a time_point
    * @param timestamp The string to parse
    * @param timeFormat The expected format (default: TIMESTAMP_FMT)
    * @return std::chrono::system_clock::time_point Parsed time point
    * @throws std::runtime_error If parsing fails
    * @note Uses the system's local time zone for conversion
    * @warning May throw on invalid input formats
    * @see formatTime() for inverse operation
    */
    std::chrono::system_clock::time_point parseTime(const std::string& timestamp, const char* timeFormat = TIMESTAMP_FMT);

    /**
     * @brief Gets the current timestamp as a string.
     * @param timeFormat The format of the timestamp.
     * @return The current timestamp as a string.
     */
    std::string getCurrentTimestamp(const char* timeFormat = TIMESTAMP_FMT);

    /**
     * @brief Converts a thread ID to a string.
     * @param id The thread ID to convert.
     * @return The string representation of the thread ID.
     */
    std::string threadIdToString(std::thread::id id);
};
#endif // !LOG_HELPER_H
