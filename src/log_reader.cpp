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
* @brief Retrieves log entries from the database matching specified filters.
* @param filters Vector of Filter objects defining search criteria.
* Each filter specifies:
* - Field name (e.g., "level", "timestamp")
* - Comparison operator ("=", ">", "<=" etc.)
* - Value to match
* - Optional type hint (for special handling)
* @param limit Maximum number of log entries to return.
* - Use -1 for no limit (default)
* - Positive values enable pagination
* @param offset Number of log entries to skip before returning results.
* - Use -1 to disable (default)
* - Requires positive limit to take effect
* @throws std::invalid_argument If filter op is empty or ivalid.
* @return LogEntryList List of log entries ordered by timestamp (descending).
*/
LogEntryList LogReader::getLogsByFilters(const std::vector<Filter> & filters,
        const int limit,
        const int offset)
{
    for(const auto & filter : filters)
    {
        if(!filter.isAllowedOp())
        {
            throw std::invalid_argument(ERR_MSG_INVALID_OPERATOR + filter.op);
        }
    }

    std::vector<std::string> fields =
    {
        FIELD_LOG_ID,
#ifdef USE_SOURCE_INFO
        FIELD_LOG_SOURCES_ID,
#endif
        FIELD_LOG_TIMESTAMP,
        FIELD_LOG_LEVEL,
        FIELD_LOG_MESSAGE,
        FIELD_LOG_FUNCTION,
        FIELD_LOG_FILE,
        FIELD_LOG_LINE,
        FIELD_LOG_THREAD_ID
    };

    // Build the query using QueryBuilder
    std::string query = QueryBuilder::buildSelect(
                            database.getDatabaseType(),
                            logsTableName,
                            fields,
                            filters,
                            FIELD_LOG_TIMESTAMP,
                            limit,
                            offset
                        );

    // Prepare parameters for the query
    std::vector<std::string> params;
    for(const auto & filter : filters)
    {
        params.push_back(filter.value);
    }

    // Execute the query
    auto result = database.query(query, params);
    LogEntryList logs;

    // Convert results to LogEntry objects
    for(const auto & row : result)
    {
#ifdef USE_SOURCE_INFO
        auto src = getSourceById(std::stoi(row.at(FIELD_LOG_SOURCES_ID)));
        bool srcHasValue = false;
        if(src.has_value() && !src.value().uuid.empty() && !src.value().name.empty())
        {
            srcHasValue = true;
        }
#endif
        logs.push_back(
        {
            std::stoi(row.at(FIELD_LOG_ID)),
#ifdef USE_SOURCE_INFO
            row.count(FIELD_LOG_SOURCES_ID) ? std::stoi(row.at(FIELD_LOG_SOURCES_ID)) : SOURCE_NOT_FOUND,
#endif
            row.at(FIELD_LOG_TIMESTAMP),
            row.at(FIELD_LOG_LEVEL),
            row.at(FIELD_LOG_MESSAGE),
            row.at(FIELD_LOG_FUNCTION),
            row.at(FIELD_LOG_FILE),
            std::stoi(row.at(FIELD_LOG_LINE)),
            row.at(FIELD_LOG_THREAD_ID)
#ifdef USE_SOURCE_INFO
            , srcHasValue ? src.value().uuid : ""
            , srcHasValue ? src.value().name : ""
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
    std::vector<Filter> filters =
    {
        {Filter::Type::SourceId, FIELD_SOURCES_ID, "=", std::to_string(sourceId)}
    };

    std::vector<std::string> fields =
    {
        FIELD_SOURCES_ID,
        FIELD_SOURCES_UUID,
        FIELD_SOURCES_NAME
    };

    std::string query = QueryBuilder::buildSelect(
                            database.getDatabaseType(),
                            SOURCES_TABLE_NAME,
                            fields,
                            filters,
                            "", // no ordering
                            1   // limit to 1 result
                        );

    auto result = database.query(query, { std::to_string(sourceId) });

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

/**
 * @brief Retrieves a source by its UUID.
 * @param uuid The UUID of the source to retrieve.
 * @return An optional containing the source information if found, or std::nullopt otherwise.
 */
std::optional<SourceInfo> LogReader::getSourceByUuid(const std::string& uuid)
{
    std::vector<Filter> filters =
    {
        {Filter::Type::Unknown, FIELD_SOURCES_UUID, "=", uuid}
    };

    std::vector<std::string> fields =
    {
        FIELD_SOURCES_ID,
        FIELD_SOURCES_UUID,
        FIELD_SOURCES_NAME
    };

    std::string query = QueryBuilder::buildSelect(
                            database.getDatabaseType(),
                            SOURCES_TABLE_NAME,
                            fields,
                            filters,
                            "", // no ordering
                            1   // limit to 1 result
                        );

    auto result = database.query(query, { uuid });

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

/**
 * @brief Retrieves a source by its name.
 * @param name The name of the source to retrieve.
 * @return An optional containing the source information if found, or std::nullopt otherwise.
 */
std::optional<SourceInfo> LogReader::getSourceByName(const std::string& name)
{
    std::vector<Filter> filters =
    {
        {Filter::Type::Unknown, FIELD_SOURCES_NAME, "=", name}
    };

    std::vector<std::string> fields =
    {
        FIELD_SOURCES_ID,
        FIELD_SOURCES_UUID,
        FIELD_SOURCES_NAME
    };

    std::string query = QueryBuilder::buildSelect(
                            database.getDatabaseType(),
                            SOURCES_TABLE_NAME,
                            fields,
                            filters,
                            "", // no ordering
                            1   // limit to 1 result
                        );

    auto result = database.query(query, { name });

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

/**
 * @brief Retrieves all sources from the database.
 * @return A vector containing all sources.
 */
std::vector<SourceInfo> LogReader::getAllSources()
{
    std::vector<std::string> fields =
    {
        FIELD_SOURCES_ID,
        FIELD_SOURCES_UUID,
        FIELD_SOURCES_NAME
    };

    std::string query = QueryBuilder::buildSelect(
                            database.getDatabaseType(),
                            SOURCES_TABLE_NAME,
                            fields,
                            {}, // no filters
                            FIELD_SOURCES_ID // order by ID
                        );

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
