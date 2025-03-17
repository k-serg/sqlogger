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

#ifndef LOG_READER_H
#define LOG_READER_H

#include <vector>
#include "log_entry.h"
#include "database_interface.h"

/**
 * @class LogReader
 * @brief Class for reading log entries from a database.
 */
class LogReader
{
    public:
        /**
         * @brief Constructs a LogReader object.
         * @param database The database interface to use for reading logs.
         */
        LogReader(IDatabase& database) : database(database) {}

        /**
         * @brief Retrieves log entries based on specified filters.
         * @param filters The filters to apply when retrieving logs.
         * @return A list of log entries that match the filters.
         */
        LogEntryList getLogsByFilters(const std::vector<Filter> & filters);

#ifdef USE_SOURCE_INFO
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
#endif

    private:
        IDatabase& database; /**< The database interface used for reading logs. */
};

#endif // LOG_READER_H