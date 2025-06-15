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

#ifndef LOG_SERIALIZER_H
#define LOG_SERIALIZER_H

#include <sstream>
#include "sqlogger/log_entry.h"

#ifdef SQLG_USE_EXTERNAL_JSON_PARSER
    #pragma message("Use external JSON parser enabled")
    #include <nlohmann/json.hpp>
#endif

namespace
{
    struct JsonParser
    {
        static std::string getString(const std::string& json, const std::string& key);
        static int getInt(const std::string& json, const std::string& key);

        /**
        * @brief Escapes special characters in a JSON string.
        * @param str The string to escape.
        * @return The escaped string.
        */
        static std::string escapeJsonString(const std::string& str);
    };
}

namespace LogSerializer
{
    namespace Json
    {
        /**
         * @brief Serializes a single log entry to JSON
         */
        std::string serializeLog(const LogEntry& entry);

        /**
         * @brief Serializes multiple logs to JSON array
         */
        std::string serializeLogs(const LogEntryList& logs);

        /**
         * @brief Serializes filter to JSON
         */
        std::string serializeFilter(const Filter& filter);

        /**
         * @brief Serializes filters to JSON
         */
        std::string serializeFilters(const std::vector<Filter> & filters);

        /**
         * @brief Parses JSON string into LogEntry
         * @throws std::runtime_error on parse failure
         */
        LogEntry parseLog(const std::string& jsonString);

        /**
        * @brief Parses JSON string into LogEntryList
        * @throws std::runtime_error on parse failure
        */
        LogEntryList parseLogs(const std::string& jsonArray);

        /**
         * @brief Parses JSON filters
         * @throws std::runtime_error on parse failure
         */
        std::vector<Filter> parseFilters(const std::string& json);

    };
};

#endif // !LOG_SERIALIZER_H
