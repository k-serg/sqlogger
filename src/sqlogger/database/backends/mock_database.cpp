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

#include "sqlogger/database/backends/mock_database.h"

/**
 * @brief Connects to the mock database
 * @param connectionString Connection string (ignored in mock implementation)
 * @return Always returns true since mock doesn't require real connection
 */
bool MockDatabase::connect(const std::string& connectionString)
{
    return true;
}

/**
 * @brief Disconnects from the mock database and clears all stored data
 */
void MockDatabase::disconnect()
{
    clearMockData();
}

/**
 * @brief Checks connection status
 * @return Always returns true for mock database
 */
bool MockDatabase::isConnected() const
{
    return true;
}

/**
 * @brief Executes an SQL command with parameters and returns affected row count
 * @param query The SQL query to execute
 * @param params Parameters to bind to the query
 * @param affectedRows Output parameter for affected rows count
 * @return Always returns true for mock database
 */
bool MockDatabase::execute(
    const std::string& query,
    const std::vector<std::string> & params,
    int* affectedRows)
{
    std::lock_guard<std::mutex> lock(mutex);
    executedQueries.push_back(query);
    executedParams.push_back(params);

    std::string processedQuery = processParameterizedQuery(query, params);

    if(processedQuery.find("DELETE") != std::string::npos)
    {
        size_t previousSize = mockLogsData.size();

        if(processedQuery.find(LOG_TABLE_NAME) != std::string::npos)
        {
            mockLogsData.clear();
        }
#ifdef SQLG_USE_SOURCE_INFO
        else if(processedQuery.find(SOURCES_TABLE_NAME) != std::string::npos)
        {
            mockSourcesData.clear();
        }
#endif

        if(affectedRows)
        {
            * affectedRows = static_cast<int>(previousSize);
        }
        return true;
    }

    if(processedQuery.find("INSERT") != std::string::npos)
    {
#ifdef SQLG_USE_SOURCE_INFO
        bool isSourcesTable = processedQuery.find(SOURCES_TABLE_NAME) != std::string::npos;
        if(isSourcesTable)
        {
            std::map<std::string, std::string> sourceEntry;
            lastInsertId++;
            sourceEntry[FIELD_SOURCES_ID] = std::to_string(lastInsertId);
            sourceEntry[FIELD_SOURCES_UUID] = params[0];
            sourceEntry[FIELD_SOURCES_NAME] = params[1];
            mockSourcesData.push_back(sourceEntry);
        }
        else
        {
            std::map<std::string, std::string> entry;
            entry[FIELD_LOG_ID] = std::to_string(mockLogsData.size() + 1);
            entry[FIELD_LOG_SOURCES_ID] = params[0];
            entry[FIELD_LOG_TIMESTAMP] = params[1];
            entry[FIELD_LOG_LEVEL] = params[2];
            entry[FIELD_LOG_MESSAGE] = params[3];
            entry[FIELD_LOG_FUNCTION] = params[4];
            entry[FIELD_LOG_FILE] = params[5];
            entry[FIELD_LOG_LINE] = params[6];
            entry[FIELD_LOG_THREAD_ID] = params[7];
            mockLogsData.push_back(entry);
        }
#else
        std::map<std::string, std::string> entry;
        entry[FIELD_LOG_ID] = std::to_string(mockLogsData.size() + 1);
        entry[FIELD_LOG_TIMESTAMP] = params[0];
        entry[FIELD_LOG_LEVEL] = params[1];
        entry[FIELD_LOG_MESSAGE] = params[2];
        entry[FIELD_LOG_FUNCTION] = params[3];
        entry[FIELD_LOG_FILE] = params[4];
        entry[FIELD_LOG_LINE] = params[5];
        entry[FIELD_LOG_THREAD_ID] = params[6];
        mockLogsData.push_back(entry);
#endif

        if(affectedRows)
        {
            * affectedRows = 1;
        }
        return true;
    }

    if(affectedRows)
    {
        * affectedRows = 0;
    }
    return true;
}

/**
 * @brief Executes a query and returns results with parameter support
 * @param query The SQL query to execute
 * @param params Parameters to bind
 * @return Query results as vector of maps
 */
std::vector<std::map<std::string, std::string>> MockDatabase::query(
    const std::string& query,
    const std::vector<std::string> & params)
{
    std::lock_guard<std::mutex> lock(mutex);
    executedQueries.push_back(query);
    executedParams.push_back(params);

    std::string processedQuery = processParameterizedQuery(query, params);

#ifdef SQLG_USE_SOURCE_INFO
    bool isSourcesTable = processedQuery.find(SOURCES_TABLE_NAME) != std::string::npos;
    const auto& data = isSourcesTable ? mockSourcesData : mockLogsData;

    if(processedQuery.find("LAST_INSERT_ID") != std::string::npos ||
            processedQuery.find("LAST_INSERT_ROWID") != std::string::npos)
    {
        std::map<std::string, std::string> resultRow;
        resultRow["LAST_INSERT_ID()"] = std::to_string(lastInsertId);
        return { resultRow };
    }
#else
    const auto& data = mockLogsData;
#endif

    std::vector<std::map<std::string, std::string>> result;

    std::vector<Filter> filters;
    size_t wherePosition = processedQuery.find("WHERE");
    if(wherePosition != std::string::npos)
    {
        std::string filterString = processedQuery.substr(wherePosition + 6);
        size_t semicolonPosition = filterString.find(';');
        if(semicolonPosition != std::string::npos)
        {
            filterString = filterString.substr(0, semicolonPosition);
        }

        size_t andPosition = 0;
        while((andPosition = filterString.find("AND")) != std::string::npos)
        {
            std::string singleFilter = filterString.substr(0, andPosition);
            parseFilter(singleFilter, filters);
            filterString = filterString.substr(andPosition + 4);
        }
        parseFilter(filterString, filters);
    }

    for(const auto & entry : data)
    {
        bool match = true;
        for(const auto & filter : filters)
        {
            if(entry.find(filter.field) == entry.end() ||
                    !applyFilter(entry.at(filter.field), filter))
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

    size_t limitPosition = processedQuery.find("LIMIT");
    if(limitPosition != std::string::npos)
    {
        std::string limitString = processedQuery.substr(limitPosition + 6);
        int limit = std::stoi(limitString);
        if(limit < result.size())
        {
            result.resize(limit);
        }
    }

    return result;
}

/**
 * @brief Begins a transaction
 * @return Always returns true for mock database
 */
bool MockDatabase::beginTransaction()
{
    return true;
}

/**
 * @brief Commits current transaction
 * @return Always returns true for mock database
 */
bool MockDatabase::commitTransaction()
{
    return true;
}

/**
 * @brief Rolls back current transaction
 * @return Always returns true for mock database
 */
bool MockDatabase::rollbackTransaction()
{
    return true;
}

/**
 * @brief Drops the mock database
 * @param connectionString Database name (ignored)
 * @return Always returns true
 */
bool MockDatabase::dropDatabaseIfExists(const std::string& connectionString)
{
    std::lock_guard<std::mutex> lock(mutex);
    executedQueries.push_back("DROP DATABASE IF EXISTS " + connectionString);
    clearMockData();
    return true;
}

/**
 * @brief Gets last error message
 * @return Empty string (mocks don't generate errors)
 */
std::string MockDatabase::getLastError() const
{
    return "";
}

/**
 * @brief Gets database type
 * @return Always returns DataBaseType::Mock
 */
DataBaseType MockDatabase::getDatabaseType() const
{
    return dbType;
}

/**
 * @brief Clears all mock data and query history
 */
void MockDatabase::clearMockData()
{
    std::lock_guard<std::mutex> lock(mutex);
    mockLogsData.clear();
#ifdef SQLG_USE_SOURCE_INFO
    mockSourcesData.clear();
    lastInsertId = 0;
#endif
    executedQueries.clear();
    executedParams.clear();
}

/**
 * @brief Gets list of executed queries
 * @return Vector of executed SQL statements
 */
std::vector<std::string> MockDatabase::getExecutedQueries() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return executedQueries;
}

/**
 * @brief Gets list of query parameters
 * @return Vector of parameter sets
 */
std::vector<std::vector<std::string>> MockDatabase::getExecutedParams() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return executedParams;
}

/**
 * @brief Gets current mock data
 * @return Vector of mock data rows
 */
std::vector<std::map<std::string, std::string>> MockDatabase::getMockData() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return mockLogsData;
}

/**
 * @brief Processes parameterized query by replacing placeholders with values
 * @param query The original query with placeholders
 * @param params Parameters to substitute
 * @return Query with parameters inserted
 */
std::string MockDatabase::processParameterizedQuery(
    const std::string& query,
    const std::vector<std::string> & params)
{
    std::string processedQuery = query;

    if(!params.empty())
    {
        if(getDatabaseType() == DataBaseType::PostgreSQL)
        {
            for(size_t index = 0; index < params.size(); ++index)
            {
                std::string placeholder = "$" + std::to_string(index + 1);
                size_t position = processedQuery.find(placeholder);
                if(position != std::string::npos)
                {
                    processedQuery.replace(position, placeholder.length(),
                                           "'" + params[index] + "'");
                }
            }
        }
        else
        {
            size_t paramIndex = 0;
            size_t position = 0;
            while((position = processedQuery.find('?', position)) != std::string::npos &&
                    paramIndex < params.size())
            {
                processedQuery.replace(position, 1, "'" + params[paramIndex] + "'");
                position += params[paramIndex].length() + 2;
                paramIndex++;
            }
        }
    }

    return processedQuery;
}

/**
 * @brief Parses a filter condition from query string
 * @param filterString The filter string to parse
 * @param filters Output vector for parsed filters
 */
void MockDatabase::parseFilter(const std::string& filterString, std::vector<Filter> & filters)
{
    size_t spacePosition = filterString.find(' ');
    if(spacePosition != std::string::npos)
    {
        std::string field = filterString.substr(0, spacePosition);
        std::string remaining = filterString.substr(spacePosition + 1);

        spacePosition = remaining.find(' ');
        if(spacePosition != std::string::npos)
        {
            std::string operation = remaining.substr(0, spacePosition);
            std::string value = remaining.substr(spacePosition + 1);

            if(value.front() == '\'')
            {
                size_t quotePosition = value.find('\'', 1);
                if(quotePosition != std::string::npos)
                {
                    value = value.substr(1, quotePosition - 1);
                }
                else
                {
                    value = value.substr(1);
                }
            }

            Filter filter;
            filter.field = field;
            filter.op = operation;
            filter.value = value;
            filter.type = filter.fieldToType();
            filters.emplace_back(filter);
        }
    }
}

/**
 * @brief Applies filter to a value
 * @param value The value to check
 * @param filter The filter to apply
 * @return true if value matches the filter condition
 */
bool MockDatabase::applyFilter(const std::string& value, const Filter& filter)
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
    return false;
}

/**
 * @brief Converts field name to type for filtering
 * @return Field type as string
 */
std::string MockDatabase::Filter::fieldToType() const
{
    return "string";
}