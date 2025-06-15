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

#include "sqlogger/log_helper.h"

/**
* @brief Joins a vector of strings into a single string with a delimiter
* @param parts The vector of strings to join
* @param delimiter The string to insert between joined parts
* @return std::string The resulting concatenated string
*/
std::string StringHelper::join(const std::vector<std::string> & parts, const std::string& delimiter)
{
    std::string result;
    for(size_t i = 0; i < parts.size(); ++i)
    {
        if(i != 0)
        {
            result += delimiter;
        }
        result += parts[i];
    }
    return result;
}

/**
* @brief Splits a string into a vector of strings using a delimiter
* @param str The string to split
* @param delimiter The string to use as delimiter
* @return std::vector<std::string> The resulting vector of strings
*/
std::vector<std::string> StringHelper::split(const std::string& str, const std::string& delimiter)
{
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);

    while(end != std::string::npos)
    {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }

    result.push_back(str.substr(start));

    return result;
}

#ifdef SQLG_USE_SOURCE_INFO
/**
 * @brief Generates a UUID.
 * @return A string representing the generated UUID.
 */
std::string LogHelper::generateUUID()
{
    const uuids::uuid uuid = uuids::uuid_system_generator{}();
    return uuids::to_string(uuid);
}
#endif

/**
* @brief Checks if std::string represents a numeric value
* @param value std::string to check
* @return True if numeric, false otherwise
*/
bool LogHelper::isNumeric(const std::string& value)
{
    if(value.empty()) return false;
    size_t start = (value[0] == '-') ? 1 : 0;
    bool has_dot = false;

    for(size_t i = start; i < value.size(); ++i)
    {
        if(value[i] == '.' && !has_dot)
        {
            has_dot = true;
        }
        else if(!std::isdigit(value[i]))
        {
            return false;
        }
    }
    return true;
}

/**
 * @brief Transform string into upper case. ASCII-only.
 * @param input Source string.
 * @return String transformed into upper case.
*/
std::string LogHelper::toUpperCase(const std::string& input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

/**
 * @brief Transform string into lower case. ASCII-only.
 * @param input Source string.
 * @return String transformed into lower case.
*/
std::string LogHelper::toLowerCase(const std::string& input)
{
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

/**
  * @brief Converts a log level to its string representation.
  * @param level The log level to convert.
  * @return The string representation of the log level.
  */
std::string LogHelper::levelToString(LogLevel level)
{
    switch(level)
    {
        case LogLevel::Trace:
            return LOG_LEVEL_TRACE;
        case LogLevel::Debug:
            return LOG_LEVEL_DEBUG;
        case LogLevel::Info:
            return LOG_LEVEL_INFO;
        case LogLevel::Warning:
            return LOG_LEVEL_WARNING;
        case LogLevel::Error:
            return LOG_LEVEL_ERROR;
        case LogLevel::Fatal:
            return LOG_LEVEL_FATAL;
        default:
            return LOG_LEVEL_UNKNOWN;
    }
}

/**
* @brief Converts a log level to its integer representation.
* @param level The log level to convert.
* @return The integer representation of the log level.
*/
int LogHelper::levelToInt(LogLevel level)
{
    switch(level)
    {
        case LogLevel::Trace:
        case LogLevel::Debug:
        case LogLevel::Info:
        case LogLevel::Warning:
            ;
        case LogLevel::Error:
        case LogLevel::Fatal:
            return static_cast<int>(level);
        default:
            return static_cast<int>(LogLevel::Unknown);
    }
}

/**
 * @brief Converts a string to a log level.
 * @param levelStr The string representation of the log level.
 * @return The corresponding log level.
 */
LogLevel LogHelper::stringToLevel(const std::string& levelStr, const bool ignoreCase)
{
    if((ignoreCase ? toUpperCase(levelStr) : levelStr) == LOG_LEVEL_TRACE) return LogLevel::Trace;
    if((ignoreCase ? toUpperCase(levelStr) : levelStr) == LOG_LEVEL_DEBUG) return LogLevel::Debug;
    if((ignoreCase ? toUpperCase(levelStr) : levelStr) == LOG_LEVEL_INFO) return LogLevel::Info;
    if((ignoreCase ? toUpperCase(levelStr) : levelStr) == LOG_LEVEL_WARNING) return LogLevel::Warning;
    if((ignoreCase ? toUpperCase(levelStr) : levelStr) == LOG_LEVEL_ERROR) return LogLevel::Error;
    if((ignoreCase ? toUpperCase(levelStr) : levelStr) == LOG_LEVEL_FATAL) return LogLevel::Fatal;
    return LogLevel::Unknown;
}

/**
 * @brief Converts a int to a log level.
 * @param levelInt The integer representation of the log level.
 * @return The corresponding log level.
 */
LogLevel LogHelper::intToLevel(const int levelInt)
{
    switch(levelInt)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            return static_cast<LogLevel>(levelInt);
        default:
            return LogLevel::Unknown;
    }
}

/**
* @brief Converts a time_point to a formatted string representation
* @param tp The time_point to format
* @param timeFormat The format string (default: TIMESTAMP_FMT)
* @return std::string Formatted time string
* @note Uses the system's local time zone for conversion
* @see TIMESTAMP_FMT for default format
*/
std::string LogHelper::formatTime(const std::chrono::system_clock::time_point& tp, const char* timeFormat)
{
    auto in_time_t = std::chrono::system_clock::to_time_t(tp);
    std::stringstream ss;
    ss << std::put_time(std::localtime( & in_time_t), TIMESTAMP_FMT);
    return ss.str();
}

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
std::chrono::system_clock::time_point LogHelper::parseTime(const std::string& timestamp, const char* timeFormat)
{
    std::tm tm = {};
    std::istringstream ss(timestamp);
    ss >> std::get_time( & tm, TIMESTAMP_FMT);

    if(ss.fail())
    {
        throw std::runtime_error("Failed to parse timestamp: " + timestamp);
    }

    return std::chrono::system_clock::from_time_t(std::mktime( & tm));
}

/**
 * @brief Gets the current timestamp as a string.
 * @param timeFormat The format of the timestamp.
 * @return The current timestamp as a string.
 */
std::string LogHelper::getCurrentTimestamp(const char* timeFormat)
{
    auto now = std::time(nullptr);
    auto tm = * std::localtime( & now);
    std::ostringstream oss;
    oss << std::put_time( & tm, timeFormat);
    return oss.str();
}

/**
 * @brief Converts a thread ID to a string.
 * @param id The thread ID to convert.
 * @return The string representation of the thread ID.
 */
std::string LogHelper::threadIdToString(std::thread::id id)
{
    std::ostringstream oss;
    oss << id;
    return oss.str();
}
