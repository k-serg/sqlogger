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

#include "postgresql_database.h"
#include <sstream>
#include <iostream>
//#include <libpq-fe.h>

/**
 * @brief Constructs PostgreSQL database connection handler
 * @param connectionString Connection string in format "host=... user=... password=... dbname=..."
 * @throws std::runtime_error If database creation or connection fails
 */
PostgreSQLDatabase::PostgreSQLDatabase(const std::string& connectionString)
    : conn(nullptr),
      allowCreateDB(DB_ALLOW_CREATE),
      transactionInProgress(false)
{
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
 * @brief Destructor - automatically rolls back active transaction and closes connection
 */
PostgreSQLDatabase::~PostgreSQLDatabase()
{
    if(transactionInProgress)
    {
        rollbackTransaction();
    }
    disconnect();
}

/**
 * @brief Establishes connection to PostgreSQL server
 * @param connectionString Connection parameters
 * @return true if connection established successfully
 */
bool PostgreSQLDatabase::connect(const std::string& connectionString)
{
    if(conn)
    {
        disconnect();
    }

    conn = PQconnectdb(connectionString.c_str());
    if(PQstatus(conn) != CONNECTION_OK)
    {
        lastError = PQerrorMessage(conn);
        return false;
    }
    return true;
}

/**
 * @brief Closes database connection and releases resources
 */
void PostgreSQLDatabase::disconnect()
{
    if(conn)
    {
        PQfinish(conn);
        conn = nullptr;
    }
    lastError.clear();
}

/**
 * @brief Checks connection status
 * @return true if connection is active
 */
bool PostgreSQLDatabase::isConnected() const
{
    return conn && PQstatus(conn) == CONNECTION_OK;
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
bool PostgreSQLDatabase::execute(
    const std::string& query,
    const std::vector<std::string> & params,
    int* affectedRows)
{
    if(!isConnected())
    {
        lastError = ERR_MSG_FAILED_NOT_CONNECTED_DB;
        return false;
    }

    lastError.clear();

    // Convert params to array of C strings
    std::vector<const char*> paramValues(params.size());
    for(size_t i = 0; i < params.size(); ++i)
    {
        paramValues[i] = params[i].c_str();
    }

    PGresult* res = PQexecParams(conn,
                                 query.c_str(),
                                 params.size(),
                                 nullptr,  // let PostgreSQL infer param types
                                 paramValues.data(),
                                 nullptr,  // param lengths (null means strings are null-terminated)
                                 nullptr,  // param formats (0=text, 1=binary)
                                 0);       // result format (0=text, 1=binary)

    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK ||
                    PQresultStatus(res) == PGRES_TUPLES_OK);

    if(success && affectedRows)
    {
        char* countStr = PQcmdTuples(res);
        * affectedRows = countStr[0] ? std::stoi(countStr) : 0;
    }

    if(!success)
    {
        lastError = PQerrorMessage(conn);
    }

    PQclear(res);
    return success;
}

/**
 * @brief Executes an SQL query and returns the result.
 * @param query The SQL query to execute.
 * @param params The parameters to bind to the query.
 * @return A vector of maps representing the query result.
 */
std::vector<std::map<std::string, std::string>> PostgreSQLDatabase::query(const std::string& query,
        const std::vector<std::string> & params)
{
    std::vector<std::map<std::string, std::string>> result;
    if(!isConnected())
    {
        lastError = ERR_MSG_FAILED_NOT_CONNECTED_DB;
        return result;
    }

    lastError.clear();

    // Convert params to array of C strings
    std::vector<const char*> paramValues(params.size());
    for(size_t i = 0; i < params.size(); ++i)
    {
        paramValues[i] = params[i].c_str();
    }

    PGresult* res = PQexecParams(conn,
                                 query.c_str(),
                                 params.size(),
                                 nullptr,  // let PostgreSQL infer param types
                                 paramValues.data(),
                                 nullptr,  // param lengths (null means strings are null-terminated)
                                 nullptr,  // param formats (0=text, 1=binary)
                                 0);       // result format (0=text, 1=binary)

    if(PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        lastError = PQerrorMessage(conn);
        PQclear(res);
        return result;
    }

    int rowCount = PQntuples(res);
    int colCount = PQnfields(res);

    for(int i = 0; i < rowCount; ++i)
    {
        std::map<std::string, std::string> row;
        for(int j = 0; j < colCount; ++j)
        {
            row[PQfname(res, j)] = PQgetvalue(res, i, j) ? PQgetvalue(res, i, j) : "NULL";
        }
        result.push_back(row);
    }

    PQclear(res);

    return result;
}

/**
 * @brief Starts database transaction
 * @return true if transaction started successfully
 */
bool PostgreSQLDatabase::beginTransaction()
{
    if(transactionInProgress)
    {
        lastError = "Transaction already in progress";
        return false;
    }

    transactionInProgress = execute("BEGIN");
    if(!transactionInProgress)
    {
        lastError = "Failed to begin transaction: " + getLastError();
    }
    return transactionInProgress;
}

/**
 * @brief Commits active transaction
 * @return true if transaction committed successfully
 */
bool PostgreSQLDatabase::commitTransaction()
{
    if(!transactionInProgress)
    {
        lastError = "No transaction in progress";
        return false;
    }

    bool success = execute("COMMIT");
    if(success)
    {
        transactionInProgress = false;
    }
    else
    {
        lastError = "Failed to commit transaction: " + getLastError();
    }
    return success;
}

/**
 * @brief Rolls back active transaction
 * @return true if transaction rolled back successfully
 */
bool PostgreSQLDatabase::rollbackTransaction()
{
    if(!transactionInProgress)
    {
        lastError = "No transaction in progress";
        return false;
    }

    bool success = execute("ROLLBACK");
    if(success)
    {
        transactionInProgress = false;
    }
    else
    {
        lastError = "Failed to rollback transaction: " + getLastError();
    }
    return success;
}

/**
 * @brief Drops database if exists (admin operation)
 * @param connectionString Original connection string
 * @return true if database dropped or doesn't exist
 */
bool PostgreSQLDatabase::dropDatabaseIfExists(const std::string& connectionString)
{
#if DB_ALLOW_DROP == 1
    auto params = parseConnectionString(connectionString);
    std::string dbName = params["dbname"];

    if(dbName.empty())
    {
        lastError = "Database name not specified";
        return false;
    }

    std::string adminConnStr = "host=" + params["host"] +
                               " user=" + params["user"] +
                               " password=" + params["password"] +
                               " dbname=template1";

    PGconn* adminConn = PQconnectdb(adminConnStr.c_str());
    if(PQstatus(adminConn) != CONNECTION_OK)
    {
        lastError = PQerrorMessage(adminConn);
        PQfinish(adminConn);
        return false;
    }

    std::string dropQuery = "DROP DATABASE IF EXISTS " + dbName;
    PGresult* res = PQexec(adminConn, dropQuery.c_str());

    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK);
    if(!success)
    {
        lastError = PQerrorMessage(adminConn);
    }

    PQclear(res);
    PQfinish(adminConn);
    return success;
#else
    lastError = "Database drop operation is not allowed";
    return false;
#endif
}

/**
 * @brief Gets last error message
 * @return Error description string
 */
std::string PostgreSQLDatabase::getLastError() const
{
    return lastError.empty() && conn ? PQerrorMessage(conn) : lastError;
}

/**
 * @brief Gets database type (PostgreSQL)
 * @return DataBaseType::PostgreSQL constant
 */
DataBaseType PostgreSQLDatabase::getDatabaseType() const
{
    return dbType;
}

/**
 * @brief Creates database if it doesn't exist (admin operation)
 * @param connectionString Original connection string
 * @return true if database exists or was created
 */
bool PostgreSQLDatabase::createDatabaseIfNotExists(const std::string& connectionString)
{
    auto params = parseConnectionString(connectionString);
    std::string dbName = params["dbname"];

    if(dbName.empty())
    {
        lastError = "Database name not specified";
        return false;
    }

    std::string adminConnStr = "host=" + params["host"] +
                               " user=" + params["user"] +
                               " password=" + params["password"] +
                               " dbname=template1";

    PGconn* adminConn = PQconnectdb(adminConnStr.c_str());
    if(PQstatus(adminConn) != CONNECTION_OK)
    {
        lastError = PQerrorMessage(adminConn);
        PQfinish(adminConn);
        return false;
    }

    std::string createQuery = "SELECT 1 FROM pg_database WHERE datname = '" + dbName + "'";
    PGresult* res = PQexec(adminConn, createQuery.c_str());

    bool dbExists = (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0);
    PQclear(res);

    if(!dbExists)
    {
        createQuery = "CREATE DATABASE " + dbName;
        res = PQexec(adminConn, createQuery.c_str());
        dbExists = (PQresultStatus(res) == PGRES_COMMAND_OK);
        if(!dbExists)
        {
            lastError = PQerrorMessage(adminConn);
        }
        PQclear(res);
    }

    PQfinish(adminConn);
    return dbExists;
}

/**
 * @brief Parses PostgreSQL connection string
 * @param connectionString String in format "key1=value1 key2=value2"
 * @return Map of connection parameters
 */
std::map<std::string, std::string> PostgreSQLDatabase::parseConnectionString(const std::string& connectionString)
{
    std::map<std::string, std::string> params;
    std::istringstream iss(connectionString);
    std::string token;

    while(std::getline(iss, token, ' '))
    {
        size_t pos = token.find('=');
        if(pos != std::string::npos)
        {
            params[token.substr(0, pos)] = token.substr(pos + 1);
        }
    }
    return params;
}
