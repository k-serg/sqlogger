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

#ifndef MOCK_DATABASE_H
#define MOCK_DATABASE_H

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <algorithm>
#include <functional>
#include "log_entry.h"
#include "database_interface.h"

/**
 * @class MockDatabase
 * @brief Mock implementation of the IDatabase interface for testing.
 */
class MockDatabase : public IDatabase
{
    public:
        /**
         * @brief Executes an SQL query.
         * @param query The SQL query to execute.
         * @return True if the query was executed successfully, false otherwise.
         */
        bool execute(const std::string& query) override
        {
            std::lock_guard<std::mutex> lock(mutex);
            executedQueries.push_back(query);

            // Handle log clearing query
            if(query.find("DELETE FROM " + std::string(LOG_TABLE_NAME)) != std::string::npos)
            {
                mockData.clear();
            }

            return true;
        }

        /**
         * @brief Executes an SQL query and returns the result.
         * @param query The SQL query to execute.
         * @return A vector of maps representing the query result.
         */
        std::vector<std::map<std::string, std::string>> query(const std::string& query) override
        {
            std::lock_guard<std::mutex> lock(mutex);
            executedQueries.push_back(query);

            // Emulate data return with filters
            std::vector<std::map<std::string, std::string>> result;

            // Parse filters from query
            std::vector<Filter> filters;
            size_t wherePos = query.find("WHERE");
            if(wherePos != std::string::npos)
            {
                std::string filterStr = query.substr(wherePos + 6); // "WHERE " is 6 characters

                // Remove trailing ';' if present
                size_t semicolonPos = filterStr.find(';');
                if(semicolonPos != std::string::npos)
                {
                    filterStr = filterStr.substr(0, semicolonPos);
                }

                // Split filters by 'AND'
                size_t andPos = 0;
                while((andPos = filterStr.find("AND")) != std::string::npos)
                {
                    std::string singleFilter = filterStr.substr(0, andPos);
                    parseFilter(singleFilter, filters); // Parse single filter
                    filterStr = filterStr.substr(andPos + 4); // "AND " is 4 characters
                }

                // Parse the last filter
                parseFilter(filterStr, filters);
            }

            // Filter data
            for(const auto & entry : mockData)
            {
                bool match = true;
                for(const auto & filter : filters)
                {
                    if(entry.find(filter.field) == entry.end() || !applyFilter(entry.at(filter.field), filter))
                    {
                        match = false;
                        break;
                    }
                }
                if(match)
                {
                    result.push_back(entry);
                }
            }

            return result;
        }

        /**
         * @brief Prepares and executes an SQL query with parameters.
         * @param query The SQL query to execute.
         * @param params The parameters to bind to the query.
         * @return True if the query was executed successfully, false otherwise.
         */
        bool MockDatabase::executeWithParams(const std::string& query, const std::vector<std::string> & params)
        {
            std::lock_guard<std::mutex> lock(mutex);
            if(executeWithParamsOverride)
            {
                return executeWithParamsOverride(query, params); // Use override if set
            }
            executedQueries.push_back(query);
            executedParams.push_back(params);

            // Emulate data insertion
            std::map<std::string, std::string> entry;
            entry[FIELD_ID] = std::to_string(mockData.size() + 1); // Unique ID
            entry[FIELD_TIMESTAMP] = params[0];
            entry[FIELD_LEVEL] = params[1];
            entry[FIELD_MESSAGE] = params[2];
            entry[FIELD_FUNCTION] = params[3];
            entry[FIELD_FILE] = params[4];
            entry[FIELD_LINE] = params[5];
            entry[FIELD_THREAD_ID] = params[6];
            mockData.push_back(entry);

            return true; // Simulate successful execution
        }

        /**
         * @brief Override for executeWithParams to simulate custom behavior.
         */
        std::function<bool(const std::string&, const std::vector<std::string> &)> executeWithParamsOverride;

        /**
         * @brief Clears mock data.
         */
        void clearMockData()
        {
            std::lock_guard<std::mutex> lock(mutex);
            mockData.clear();
            executedQueries.clear();
            executedParams.clear();
        }

        /**
         * @brief Gets the list of executed queries.
         * @return The list of executed queries.
         */
        std::vector<std::string> getExecutedQueries() const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return executedQueries;
        }

        /**
         * @brief Gets the list of executed parameters.
         * @return The list of executed parameters.
         */
        std::vector<std::vector<std::string>> getExecutedParams() const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return executedParams;
        }

        /**
         * @brief Gets the mock data.
         * @return The mock data.
         */
        std::vector<std::map<std::string, std::string>> getMockData() const
        {
            std::lock_guard<std::mutex> lock(mutex);
            return mockData;
        }

    private:
        /**
         * @brief Parses a filter from a query string.
         * @param filterStr The filter string to parse.
         * @param filters The vector of filters to populate.
         */
        void parseFilter(const std::string& filterStr, std::vector<Filter> & filters)
        {
            size_t spacePos = filterStr.find(' ');
            if(spacePos != std::string::npos)
            {
                std::string field = filterStr.substr(0, spacePos);
                std::string remaining = filterStr.substr(spacePos + 1);

                spacePos = remaining.find(' ');
                if(spacePos != std::string::npos)
                {
                    std::string op = remaining.substr(0, spacePos);
                    std::string value = remaining.substr(spacePos + 1);

                    if(value.front() == '\'')
                    {
                        size_t quotePos = value.find('\'', 1);
                        if(quotePos != std::string::npos)
                        {
                            value = value.substr(1, quotePos - 1);
                        }
                        else
                        {
                            value = value.substr(1);
                        }
                    }
                    else
                    {
                        // Если значение не в кавычках, берем до следующего пробела
                        size_t nextSpace = value.find(' ');
                        if(nextSpace != std::string::npos)
                        {
                            value = value.substr(0, nextSpace);
                        }
                    }

                    Filter flt;
                    flt.field = field;
                    flt.op = op;
                    flt.value = value;
                    flt.type = flt.fieldToType();
                    filters.emplace_back(flt);
                }
            }
        }

        /**
         * @brief Applies a filter to a value.
         * @param value The value to apply the filter to.
         * @param filter The filter to apply.
         * @return True if the value matches the filter, false otherwise.
         */
        bool applyFilter(const std::string& value, const Filter& filter)
        {
            if(filter.op == "=")
            {
                return value == filter.value;
            }
            else if(filter.op == ">=")
            {
                return value >= filter.value;
            }
            else if(filter.op == "<=")
            {
                return value <= filter.value;
            }
            else if(filter.op == ">")
            {
                return value > filter.value;
            }
            else if(filter.op == "<")
            {
                return value < filter.value;
            }
            return false; // Unknown operator
        }

        std::vector<std::string> executedQueries; /**< List of executed queries. */
        std::vector<std::vector<std::string>> executedParams; /**< List of executed parameters. */
        std::vector<std::map<std::string, std::string>> mockData; /**< Mock data for testing. */
        mutable std::mutex mutex; /**< Mutex for thread safety. */
};

#endif // MOCK_DATABASE_H