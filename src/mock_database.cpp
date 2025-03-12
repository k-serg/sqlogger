#include "mock_database.h"

/**
 * @brief Connects to the database.
 * @param connectionString The connection string or parameters for the database.
 * @return True if the connection was successful, false otherwise.
 */
bool MockDatabase::connect(const std::string& connectionString)
{
    return true; // Simulate successful connection
}

/**
 * @brief Disconnects from the database.
 */
void MockDatabase::disconnect()
{
    // Simulate disconnection
}

/**
 * @brief Checks if the database is connected.
 * @return True if the database is connected, false otherwise.
 */
bool MockDatabase::isConnected() const
{
    return true; // Simulate being connected
}

/**
 * @brief Executes an SQL query.
 * @param query The SQL query to execute.
 * @return True if the query was executed successfully, false otherwise.
 */
bool MockDatabase::execute(const std::string& query)
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
std::vector<std::map<std::string, std::string>> MockDatabase::query(const std::string& query)
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
 * @brief Executes an SQL query and returns the number of affected rows.
 * @param query The SQL query to execute.
 * @return The number of affected rows, or -1 if an error occurred.
 */
int MockDatabase::executeWithRowCount(const std::string& query)
{
    std::lock_guard<std::mutex> lock(mutex);
    executedQueries.push_back(query);
    return mockData.size(); // Simulate affected rows
}

/**
 * @brief Begins a transaction.
 * @return True if the transaction was started successfully, false otherwise.
 */
bool MockDatabase::beginTransaction()
{
    return true; // Simulate successful transaction start
}

/**
 * @brief Commits the current transaction.
 * @return True if the transaction was committed successfully, false otherwise.
 */
bool MockDatabase::commitTransaction()
{
    return true; // Simulate successful commit
}

/**
 * @brief Rolls back the current transaction.
 * @return True if the transaction was rolled back successfully, false otherwise.
 */
bool MockDatabase::rollbackTransaction()
{
    return true; // Simulate successful rollback
}

/**
* @brief Drops the database if it exists.
* @return True if the database was successfully dropped, false otherwise.
*/
bool MockDatabase::dropDatabaseIfExists(const std::string& dbName)
{
    return true; // Simulate successful drop
};

/**
 * @brief Gets the last error message.
 * @return The last error message as a string.
 */
std::string MockDatabase::getLastError() const
{
    return ""; // Simulate no error
}

/**
 * @brief Gets the type of the database.
 * @return The database type.
 */
DataBaseType MockDatabase::getDatabaseType() const
{
    return dbType;
}

/**
 * @brief Clears mock data.
 */
void MockDatabase::clearMockData()
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
std::vector<std::string> MockDatabase::getExecutedQueries() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return executedQueries;
}

/**
 * @brief Gets the list of executed parameters.
 * @return The list of executed parameters.
 */
std::vector<std::vector<std::string>> MockDatabase::getExecutedParams() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return executedParams;
}

/**
 * @brief Gets the mock data.
 * @return The mock data.
 */
std::vector<std::map<std::string, std::string>> MockDatabase::getMockData() const
{
    std::lock_guard<std::mutex> lock(mutex);
    return mockData;
}

/**
 * @brief Determines the type of the field.
 * @return The type of the field as a string.
 */
std::string MockDatabase::Filter::fieldToType() const
{
    // Example implementation, adjust as needed
    return "string"; // or another type depending on the field
}

/**
 * @brief Parses a filter from a query string.
 * @param filterStr The filter string to parse.
 * @param filters The vector of filters to populate.
 */
void MockDatabase::parseFilter(const std::string& filterStr, std::vector<Filter> & filters)
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
    return false; // Unknown operator
}
