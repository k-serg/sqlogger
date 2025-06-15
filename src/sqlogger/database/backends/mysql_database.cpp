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

#include "sqlogger/database/backends/mysql_database.h"

/**
 * @brief Constructs a MySQLDatabase object.
 * @throws std::runtime_error if the MySQL library cannot be initialized.
 */
MySQLDatabase::MySQLDatabase(const std::string& connectionString) : conn(nullptr), allowCreateDB(DB_ALLOW_CREATE)
{
    conn = mysql_init(nullptr);
    if(!conn)
    {
        throw std::runtime_error(ERR_MSG_MYSQL_INIT_FAILED);
    }

    if(allowCreateDB)
    {
        if(!createDatabaseIfNotExists(connectionString))
        {
            throw std::runtime_error(ERR_MSG_FAILED_CREATE_DB + getLastError());
        }
    }

    if(!connect(connectionString))
    {
        throw std::runtime_error(ERR_MSG_CONNECTION_FAILED + getLastError());
    }
}

/**
 * @brief Creates a database if it does not already exist.
 * @param connectionString The connection string containing database parameters.
 * @return True if the database was created or already exists, false otherwise.
 */
bool MySQLDatabase::createDatabaseIfNotExists(const std::string& connectionString)
{
    auto params = parseConnectionString(connectionString);

    if(!mysql_real_connect(conn, params[CON_STR_HOST].c_str(), params[CON_STR_USER].c_str(),
                           params[CON_STR_PASS].c_str(), nullptr, std::stoi(params[CON_STR_PORT].c_str()), nullptr, 0))
    {
        mysql_close(conn);
        return false;
    }

    const std::string dbName = params[CON_STR_DB].c_str();

    std::string createDbQuery = "CREATE DATABASE IF NOT EXISTS " + dbName + ";";
    if(!execute(createDbQuery.c_str()))
    {
        mysql_close(conn);
        return false;
    }

    disconnect();
    return true;
}

/**
 * @brief Drops the database if it exists.
 *
 * This method attempts to drop the database specified in the connection string.
 * It first disconnects from any existing database connection, initializes a new
 * MySQL connection, and then executes a `DROP DATABASE IF EXISTS` query.
 *
 * @param connectionString The connection string containing database parameters.
 *                         The string should be in the format:
 *                         "host=...;user=...;pass=...;database=...;port=...".
 *
 * @return true if the database was successfully dropped, false otherwise.
 *
 * @note This method is only executed if `DB_ALLOW_DROP` is set to 1.
 *       If `DB_ALLOW_DROP` is 0, the method will return false and set
 *       `errorMessage` to indicate that dropping the database is not allowed.
 *
 * @warning Dropping a database is a destructive operation and cannot be undone.
 *          Use this method with caution.
 */
bool MySQLDatabase::dropDatabaseIfExists(const std::string& connectionString)
{
#if DB_ALLOW_DROP == 1
    disconnect();

    conn = mysql_init(nullptr);
    if(!conn)
    {
        throw std::runtime_error(ERR_MSG_MYSQL_INIT_FAILED);
    }

    auto params = parseConnectionString(connectionString);

    if(!mysql_real_connect(conn, params[CON_STR_HOST].c_str(), params[CON_STR_USER].c_str(),
                           params[CON_STR_PASS].c_str(), nullptr, std::stoi(params[CON_STR_PORT].c_str()), nullptr, 0))
    {
        disconnect();
        return false;
    }

    const std::string dbName = params[CON_STR_DB];

    std::string dropDbQuery = "DROP DATABASE IF EXISTS " + dbName + ";";
    if(!execute(dropDbQuery.c_str()))
    {
        disconnect();
        return false;
    }

    disconnect();
    return true;
#else
    std::cerr << ERR_MSG_DROP_NOT_ALLOWED << std::endl;
    return false;
#endif
}

/**
 * @brief Destructor for MySQLDatabase.
 * Closes the database connection.
 */
MySQLDatabase::~MySQLDatabase()
{
    disconnect();
}

/**
 * @brief Connects to the MySQL database.
 * @param connString The connection string in the format "host=...;user=...;password=...;database=...".
 * @return True if the connection was successful, false otherwise.
 */
bool MySQLDatabase::connect(const std::string& connectionString)
{
    if(!conn)
    {
        conn = mysql_init(nullptr);
        if(!conn)
        {
            throw std::runtime_error(ERR_MSG_MYSQL_INIT_FAILED);
        }
    }

    auto params = parseConnectionString(connectionString);

    if(!mysql_real_connect(conn, params[CON_STR_HOST].c_str(), params[CON_STR_USER].c_str(),
                           params[CON_STR_PASS].c_str(), params[CON_STR_DB].c_str(), std::stoi(params[CON_STR_PORT].c_str()), nullptr, 0))
    {
        return false;
    }

    return true;
}

/**
 * @brief Disconnects from the MySQL database.
 */
void MySQLDatabase::disconnect()
{
    if(conn)
    {
        mysql_close(conn);
        conn = nullptr;
    }
}

/**
 * @brief Checks if the database is connected.
 * @return True if the database is connected, false otherwise.
 */
bool MySQLDatabase::isConnected() const
{
    return conn != nullptr;
}

/**
* @brief Executes an SQL query with optional parameters and returns affected row count
*
* @param query The SQL query to execute. Can contain parameter placeholders
* @param params Vector of parameter values to bind to the query (default empty)
* @param affectedRows Optional pointer to store number of affected rows (default nullptr)
* @return true if query executed successfully
* @return false if execution failed (check getLastError() for details)
*
* @note For parameterized queries:
* - MySQL/SQLite use '?' placeholders
* - PostgreSQL uses '$1', '$2', etc. placeholders
* - Parameters are bound in order they appear in the query
*
* @note The affectedRows parameter will contain:
* - For INSERT/UPDATE/DELETE: Number of rows modified
* - For other statements: 0 or implementation-defined value
* - Only updated if pointer is not null
*
* @see getLastError()
*/
bool MySQLDatabase::execute(
    const std::string& query,
    const std::vector<std::string> & params,
    int* affectedRows)
{
    if(params.empty())
    {
        // Simple query execution without parameters
        if(mysql_query(conn, query.c_str()))
        {
            return false;
        }

        if(affectedRows)
        {
            * affectedRows = mysql_affected_rows(conn);
        }
        return true;
    }

    // Parameterized query execution
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if(!stmt)
    {
        return false;
    }

    if(mysql_stmt_prepare(stmt, query.c_str(), query.size()))
    {
        mysql_stmt_close(stmt);
        return false;
    }

    // Bind parameters
    std::vector<MYSQL_BIND> binds(params.size());
    std::vector<std::vector<char>> buffers(params.size());

    for(size_t i = 0; i < params.size(); ++i)
    {
        buffers[i].assign(params[i].begin(), params[i].end());
        buffers[i].push_back('\0');

        binds[i].buffer_type = MYSQL_TYPE_STRING;
        binds[i].buffer = buffers[i].data();
        binds[i].buffer_length = params[i].length();
        binds[i].is_null = nullptr;
        binds[i].length = & binds[i].buffer_length;
    }

    if(mysql_stmt_bind_param(stmt, binds.data()))
    {
        mysql_stmt_close(stmt);
        return false;
    }

    if(mysql_stmt_execute(stmt))
    {
        mysql_stmt_close(stmt);
        return false;
    }

    if(affectedRows)
    {
        * affectedRows = mysql_stmt_affected_rows(stmt);
    }

    mysql_stmt_close(stmt);
    return true;
}

/**
 * @brief Executes an SQL query and returns the result.
 * @param query The SQL query to execute.
 * @param params The parameters to bind to the query.
 * @return A vector of maps representing the query result.
 */
std::vector<std::map<std::string, std::string>> MySQLDatabase::query(
    const std::string& query,
    const std::vector<std::string> & params)
{
    std::vector<std::map<std::string, std::string>> result;

    if(!params.empty())
    {
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if(!stmt)
        {
            return result;
        }

        if(mysql_stmt_prepare(stmt, query.c_str(), query.size()) != 0)
        {
            mysql_stmt_close(stmt);
            return result;
        }

        // Bind parameters
        std::vector<MYSQL_BIND> binds(params.size());
        std::vector<std::vector<char>> buffers(params.size());

        for(size_t i = 0; i < params.size(); ++i)
        {
            buffers[i].assign(params[i].begin(), params[i].end());
            buffers[i].push_back('\0');

            binds[i].buffer_type = MYSQL_TYPE_STRING;
            binds[i].buffer = buffers[i].data();
            binds[i].buffer_length = params[i].length();
            binds[i].is_null = nullptr;
            binds[i].length = & binds[i].buffer_length;
        }

        if(mysql_stmt_bind_param(stmt, binds.data()) != 0)
        {
            mysql_stmt_close(stmt);
            return result;
        }

        if(mysql_stmt_execute(stmt) != 0)
        {
            mysql_stmt_close(stmt);
            return result;
        }

        // Store result to enable row counting
        if(mysql_stmt_store_result(stmt) != 0)
        {
            mysql_stmt_close(stmt);
            return result;
        }

        MYSQL_RES* meta = mysql_stmt_result_metadata(stmt);
        if(!meta)
        {
            mysql_stmt_close(stmt);
            return result;
        }

        int numFields = mysql_num_fields(meta);
        std::vector<MYSQL_BIND> result_binds(numFields);
        std::vector<std::vector<char>> result_buffers(numFields);
        std::vector<unsigned long> lengths(numFields);

        MYSQL_FIELD* fields = mysql_fetch_fields(meta);
        for(int i = 0; i < numFields; ++i)
        {
            result_buffers[i].resize(fields[i].max_length > 0 ? fields[i].max_length + 1 : 256);

            result_binds[i].buffer_type = MYSQL_TYPE_STRING;
            result_binds[i].buffer = result_buffers[i].data();
            result_binds[i].buffer_length = result_buffers[i].size();
            result_binds[i].length = & lengths[i];
            result_binds[i].is_null = nullptr;
        }

        if(mysql_stmt_bind_result(stmt, result_binds.data()) != 0)
        {
            mysql_free_result(meta);
            mysql_stmt_close(stmt);
            return result;
        }

        while(mysql_stmt_fetch(stmt) == 0)
        {
            std::map<std::string, std::string> rowData;
            for(int i = 0; i < numFields; ++i)
            {
                std::string value(result_buffers[i].data(), lengths[i]);
                rowData[fields[i].name] = value;
            }
            result.push_back(rowData);
        }

        mysql_free_result(meta);
        mysql_stmt_close(stmt);
    }
    else
    {
        if(mysql_query(conn, query.c_str()) != 0)
        {
            return result;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if(!res)
        {
            return result;
        }

        int numFields = mysql_num_fields(res);
        MYSQL_ROW row;

        while((row = mysql_fetch_row(res)))
        {
            std::map<std::string, std::string> rowData;
            for(int i = 0; i < numFields; ++i)
            {
                MYSQL_FIELD* field = mysql_fetch_field_direct(res, i);
                rowData[field->name] = row[i] ? row[i] : "NULL";
            }
            result.push_back(rowData);
        }

        mysql_free_result(res);
    }

    return result;
}

/**
 * @brief Begins a transaction.
 * @return True if the transaction was started successfully, false otherwise.
 */
bool MySQLDatabase::beginTransaction()
{
    return execute("START TRANSACTION;");
}

/**
 * @brief Commits the current transaction.
 * @return True if the transaction was committed successfully, false otherwise.
 */
bool MySQLDatabase::commitTransaction()
{
    return execute("COMMIT;");
}

/**
 * @brief Rolls back the current transaction.
 * @return True if the transaction was rolled back successfully, false otherwise.
 */
bool MySQLDatabase::rollbackTransaction()
{
    return execute("ROLLBACK;");
}

/**
 * @brief Gets the last error message.
 * @return The last error message as a string.
 */
std::string MySQLDatabase::getLastError() const
{
    if(conn)
    {
        return mysql_error(conn);
    }
    return ERR_MSG_FAILED_NOT_CONNECTED_DB;
}

/**
 * @brief Gets the type of the database.
 * @return The database type (MySQL in this case).
 */
DataBaseType MySQLDatabase::getDatabaseType() const
{
    return dbType;
}

/**
 * @brief Parses a MySQL connection string.
 * @param connString The connection string in the format "host=...;user=...;password=...;database=...".
 * @return A map containing the connection parameters.
 */
std::map<std::string, std::string> MySQLDatabase::parseConnectionString(const std::string& connectionString)
{
    std::map<std::string, std::string> params;
    std::istringstream iss(connectionString);
    std::string token;

    while(std::getline(iss, token, ';'))
    {
        size_t pos = token.find('=');
        if(pos != std::string::npos)
        {
            std::string key = token.substr(0, pos);
            std::string value = token.substr(pos + 1);
            params[key] = value;
        }
    }

    return params;
}
