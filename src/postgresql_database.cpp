/**
 * @file postgresql_database.cpp
 * @brief PostgreSQL database implementation for SQLogger system
 * @author Sergey K <sergey[no_spam]@greenblit.com>
 * @copyright GNU General Public License v3.0
 */

#include "postgresql_database.h"
#include <sstream>
#include <iostream>
#include <libpq-fe.h>

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
 * @brief Executes SQL query without result set
 * @param query SQL query to execute
 * @return true if query executed successfully
 */
bool PostgreSQLDatabase::execute(const std::string& query)
{
    if(!isConnected())
    {
        lastError = ERR_MSG_FAILED_NOT_CONNECTED_DB;
        return false;
    }

    lastError.clear();
    PGresult* res = PQexec(conn, query.c_str());

    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK ||
                    (PQresultStatus(res) == PGRES_TUPLES_OK));

    if(!success)
    {
        lastError = PQerrorMessage(conn);
    }

    PQclear(res);
    return success;
}

/**
 * @brief Executes SQL query and returns result set
 * @param query SQL query to execute
 * @return Vector of rows, where each row is a map of column-value pairs
 */
std::vector<std::map<std::string, std::string>> PostgreSQLDatabase::query(const std::string& query)
{
    std::vector<std::map<std::string, std::string>> result;
    if(!isConnected())
    {
        lastError = ERR_MSG_FAILED_NOT_CONNECTED_DB;
        return result;
    }

    lastError.clear();
    PGresult* res = PQexec(conn, query.c_str());

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
 * @brief Executes parameterized SQL query
 * @param query SQL query with placeholders (?)
 * @param params Vector of parameter values
 * @return true if query executed successfully
 */
bool PostgreSQLDatabase::executeWithParams(const std::string& query,
        const std::vector<std::string> & params)
{
    if(!isConnected())
    {
        lastError = ERR_MSG_FAILED_NOT_CONNECTED_DB;
        return false;
    }

    lastError.clear();
    std::vector<const char*> paramValues(params.size());
    for(size_t i = 0; i < params.size(); ++i)
    {
        paramValues[i] = params[i].c_str();
    }

    PGresult* res = PQexecParams(conn, query.c_str(),
                                 params.size(),
                                 nullptr,
                                 paramValues.data(),
                                 nullptr,
                                 nullptr,
                                 0);

    bool success = (PQresultStatus(res) == PGRES_COMMAND_OK ||
                    (PQresultStatus(res) == PGRES_TUPLES_OK));

    if(!success)
    {
        lastError = PQerrorMessage(conn);
    }

    PQclear(res);
    return success;
}

/**
 * @brief Executes SQL query and returns affected row count
 * @param query SQL query to execute
 * @return Number of affected rows or -1 on error
 */
int PostgreSQLDatabase::executeWithRowCount(const std::string& query)
{
    if(!isConnected())
    {
        lastError = ERR_MSG_FAILED_NOT_CONNECTED_DB;
        return -1;
    }

    lastError.clear();
    PGresult* res = PQexec(conn, query.c_str());

    if(PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        lastError = PQerrorMessage(conn);
        PQclear(res);
        return -1;
    }

    char* countStr = PQcmdTuples(res);
    int rowCount = countStr[0] ? std::stoi(countStr) : 0;

    PQclear(res);
    return rowCount;
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
