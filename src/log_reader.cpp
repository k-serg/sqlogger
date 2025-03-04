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
        logs.push_back(LogEntry
        {
            std::stoi(row.at(FIELD_ID)),
            row.at(FIELD_TIMESTAMP),
               row.at(FIELD_LEVEL),
               row.at(FIELD_MESSAGE),
               row.at(FIELD_FUNCTION),
               row.at(FIELD_FILE),
               std::stoi(row.at(FIELD_LINE)),
               row.at(FIELD_THREAD_ID)
        });
    }
    return logs;
}