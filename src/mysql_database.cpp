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

#include "mysql_database.h"

/**
 * @brief Constructs a MySQLDatabase object.
 * @throws std::runtime_error if the MySQL library cannot be initialized.
 */
MySQLDatabase::MySQLDatabase(const std::string& connectionString) : conn(nullptr), dbType(DataBaseType::MySQL), allowCreateDB(DB_ALLOW_CREATE)
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
 * @brief Executes an SQL query.
 * @param query The SQL query to execute.
 * @return True if the query was executed successfully, false otherwise.
 */
bool MySQLDatabase::execute(const std::string& query)
{
    if(mysql_query(conn, query.c_str()) != 0)
    {
        return false;
    }
    return true;
}

/**
 * @brief Executes an SQL query and returns the result.
 * @param query The SQL query to execute.
 * @return A vector of maps representing the query result.
 */
std::vector<std::map<std::string, std::string>> MySQLDatabase::query(const std::string& query)
{
    std::vector<std::map<std::string, std::string>> result;

    const std::string escapedQuery = DataBaseHelper::escapeBackslashes(query);

    if(mysql_query(conn, escapedQuery.c_str()) != 0)
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
    return result;
}

/**
 * @brief Prepares and executes an SQL query with parameters.
 * @param query The SQL query to execute.
 * @param params The parameters to bind to the query.
 * @return True if the query was executed successfully, false otherwise.
 */
bool MySQLDatabase::executeWithParams(const std::string& query, const std::vector<std::string> & params)
{
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if(!stmt)
    {
        return false;
    }

    if(mysql_stmt_prepare(stmt, query.c_str(), query.size()) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }

    std::vector<MYSQL_BIND> binds(params.size());
    for(size_t i = 0; i < params.size(); ++i)
    {
        binds[i].buffer_type = MYSQL_TYPE_STRING;
        binds[i].buffer = const_cast<char*>(params[i].c_str());
        binds[i].buffer_length = params[i].size();
    }

    if(mysql_stmt_bind_param(stmt, binds.data()) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }

    if(mysql_stmt_execute(stmt) != 0)
    {
        mysql_stmt_close(stmt);
        return false;
    }

    mysql_stmt_close(stmt);
    return true;
}

/**
 * @brief Executes an SQL query and returns the number of affected rows.
 * @param query The SQL query to execute.
 * @return The number of affected rows, or -1 if an error occurred.
 */
int MySQLDatabase::executeWithRowCount(const std::string& query)
{
    if(mysql_query(conn, query.c_str()) != 0)
    {
        return -1;
    }
    return mysql_affected_rows(conn);
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
    return ERR_MSG_DB_NOT_CONNECTED;
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
