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

#include "log_reader.h"

/**
 * @brief Retrieves log entries based on specified filters.
 * @param filters The filters to apply when retrieving logs.
 * @return A list of log entries that match the filters.
 */
LogEntryList LogReader::getLogsByFilters(const std::vector<Filter> & filters)
{
    std::string query = "SELECT * FROM " + std::string(LOG_TABLE_NAME);
    if(!filters.empty())
    {
        query += " WHERE ";
        for(size_t i = 0; i < filters.size(); ++i)
        {
            if(i > 0) query += " AND ";
            query += filters[i].field + " " + filters[i].op + " '" + filters[i].value + "'";
        }
    }
    query += ";";

    auto result = database.query(query);
    LogEntryList logs;
    for(const auto & row : result)
    {
#ifdef USE_SOURCE_INFO
        auto src = getSourceById(std::stoi(row.at(FIELD_LOG_SOURCES_ID)));
        bool srcHasValue = false;
        if (src.has_value() && !src.value().uuid.empty() && !src.value().name.empty())
        {
            srcHasValue = true;
        }
#endif
        logs.push_back(LogEntry
            {
                std::stoi(row.at(FIELD_LOG_ID)),
    #ifdef USE_SOURCE_INFO
                std::stoi(row.at(FIELD_LOG_SOURCES_ID)),
    #endif
                row.at(FIELD_LOG_TIMESTAMP),
                   row.at(FIELD_LOG_LEVEL),
                   row.at(FIELD_LOG_MESSAGE),
                   row.at(FIELD_LOG_FUNCTION),
                   row.at(FIELD_LOG_FILE),
                   std::stoi(row.at(FIELD_LOG_LINE)),
                   row.at(FIELD_LOG_THREAD_ID)
#ifdef USE_SOURCE_INFO
                   , srcHasValue ? (src.value().uuid) : ""
                   , srcHasValue ? (src.value().name) : ""
#endif
        });
    }
    return logs;
}

#ifdef USE_SOURCE_INFO
/**
 * @brief Retrieves a source by its source ID.
 * @param sourceId The source ID of the source to retrieve.
 * @return An optional containing the source information if found, or std::nullopt otherwise.
 */
std::optional<SourceInfo> LogReader::getSourceById(const int sourceId)
{
    std::string query = "SELECT * FROM " + std::string(SOURCES_TABLE_NAME) + " WHERE " + std::string(FIELD_SOURCES_ID) + " = " + std::to_string(sourceId) + ";";
    auto result = database.query(query);

    if(result.empty())
    {
        return std::nullopt;
    }

    const auto& row = result[0];
    return SourceInfo
    {
        std::stoi(row.at(FIELD_SOURCES_ID)),
        row.at(FIELD_SOURCES_UUID),
           row.at(FIELD_SOURCES_NAME)
    };
}
#endif

#ifdef USE_SOURCE_INFO
/**
 * @brief Retrieves a source by its UUID.
 * @param uuid The UUID of the source to retrieve.
 * @return An optional containing the source information if found, or std::nullopt otherwise.
 */
std::optional<SourceInfo> LogReader::getSourceByUuid(const std::string& uuid)
{
    std::string query = "SELECT * FROM " + std::string(SOURCES_TABLE_NAME) + " WHERE " + std::string(FIELD_SOURCES_UUID) + " = '" + uuid + "';";
    auto result = database.query(query);

    if(result.empty())
    {
        return std::nullopt;
    }

    const auto& row = result[0];
    return SourceInfo
    {
        std::stoi(row.at(FIELD_SOURCES_ID)),
        row.at(FIELD_SOURCES_UUID),
           row.at(FIELD_SOURCES_NAME)
    };
}
#endif

#ifdef USE_SOURCE_INFO
/**
 * @brief Retrieves a source by its name.
 * @param name The name of the source to retrieve.
 * @return An optional containing the source information if found, or std::nullopt otherwise.
 */
std::optional<SourceInfo> LogReader::getSourceByName(const std::string& name)
{
    std::string query = "SELECT * FROM " + std::string(SOURCES_TABLE_NAME) + " WHERE " + std::string(FIELD_SOURCES_NAME) + " = '" + name + "';";
    auto result = database.query(query);

    if(result.empty())
    {
        return std::nullopt;
    }

    const auto& row = result[0];
    return SourceInfo
    {
        std::stoi(row.at(FIELD_SOURCES_ID)),
        row.at(FIELD_SOURCES_UUID),
           row.at(FIELD_SOURCES_NAME)
    };
}
#endif

#ifdef USE_SOURCE_INFO
/**
 * @brief Retrieves all sources from the database.
 * @return A vector containing all sources.
 */
std::vector<SourceInfo> LogReader::getAllSources()
{
    std::string query = "SELECT * FROM " + std::string(SOURCES_TABLE_NAME) + ";";
    auto result = database.query(query);
    std::vector<SourceInfo> sources;

    for(const auto & row : result)
    {
        sources.push_back(SourceInfo
        {
            std::stoi(row.at(FIELD_SOURCES_ID)),
            row.at(FIELD_SOURCES_UUID),
               row.at(FIELD_SOURCES_NAME)
        });
    }

    return sources;
}
#endif

