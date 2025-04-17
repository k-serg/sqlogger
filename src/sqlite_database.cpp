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

#include <iostream>
#include "sqlite_database.h"
#include "fs_helper.h"
#include "log_strings.h"

/**
 * @brief Constructs an SQLiteDatabase object.
 * @param dbPath The path to the SQLite database file.
 * @throws std::runtime_error if the database cannot be opened.
 */
SQLiteDatabase::SQLiteDatabase(const std::string& dbPath) : dbPath(dbPath), db(nullptr)
{
    std::string errMsg;
    if(!FSHelper::createDir(dbPath, errMsg))
    {
        throw std::runtime_error(ERR_MSG_FAILED_CREATE_DIR + errMsg);
    }

    if(!createDatabaseIfNotExists(dbPath))
    {
        throw std::runtime_error(ERR_MSG_FAILED_CREATE_DB + getLastError());
    }

    if(!connect(dbPath))
    {
        throw std::runtime_error(ERR_MSG_FAILED_OPEN_DB + dbPath);
    }
}

/**
 * @brief Destructor for SQLiteDatabase.
 * Closes the database connection.
 */
SQLiteDatabase::~SQLiteDatabase()
{
    disconnect();
}

/**
 * @brief Creates a database if it does not already exist.
 * @param dbPath The path to the SQLite database file.
 * @return True if the database was created or already exists, false otherwise.
 */
bool SQLiteDatabase::createDatabaseIfNotExists(const std::string& dbPath)
{
    if(sqlite3_open(dbPath.c_str(), & db) != SQLITE_OK)
    {
        return false;
    }
    return true;
}

/**
 * @brief Connects to the SQLite database.
 * @param path The path to the SQLite database file.
 * @return True if the connection was successful, false otherwise.
 */
bool SQLiteDatabase::connect(const std::string& path)
{
    if(db)
    {
        sqlite3_close(db);
    }

    if(sqlite3_open(path.c_str(), & db) != SQLITE_OK)
    {
        return false;
    }

#if USE_WAL_MODE == 1
    if(!execute("PRAGMA journal_mode=WAL;"))
    {
        return false;
    }
#endif

    return true;
}

/**
 * @brief Disconnects from the SQLite database.
 */
void SQLiteDatabase::disconnect()
{
    if(db)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

/**
 * @brief Checks if the database is connected.
 * @return True if the database is connected, false otherwise.
 */
bool SQLiteDatabase::isConnected() const
{
    return db != nullptr;
}

/**
 * @brief Executes an SQL query.
 * @param query The SQL query to execute.
 * @return True if the query was executed successfully, false otherwise.
 */
bool SQLiteDatabase::execute(const std::string& query)
{
    char* errMsg = nullptr;
    if(sqlite3_exec(db, query.c_str(), nullptr, nullptr, & errMsg) != SQLITE_OK)
    {
        std::cerr << ERR_MSG_SQL_ERR << errMsg << std::endl;
        sqlite3_free(errMsg);
        reconnect();
        return false;
    }
    return true;
}

/**
 * @brief Executes an SQL query and returns the result.
 * @param query The SQL query to execute.
 * @return A vector of maps representing the query result.
 */
std::vector<std::map<std::string, std::string>> SQLiteDatabase::query(const std::string& query)
{
    std::vector<std::map<std::string, std::string>> result;
    sqlite3_stmt* stmt;

    if(sqlite3_prepare_v2(db, query.c_str(), -1, & stmt, nullptr) == SQLITE_OK)
    {
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::map<std::string, std::string> row;
            int columnCount = sqlite3_column_count(stmt);

            for(int i = 0; i < columnCount; ++i)
            {
                const char* columnName = sqlite3_column_name(stmt, i);
                const char* columnValue = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                row[columnName] = columnValue ? columnValue : "";
            }

            result.push_back(row);
        }

        sqlite3_finalize(stmt);
    }
    else
    {
        std::cerr << ERR_MSG_FAILED_QUERY << ": " << sqlite3_errmsg(db) << std::endl;
    }

    return result;
}

/**
 * @brief Prepares and executes an SQL query with parameters.
 * @param query The SQL query to execute.
 * @param params The parameters to bind to the query.
 * @return True if the query was executed successfully, false otherwise.
 */
bool SQLiteDatabase::executeWithParams(const std::string& query, const std::vector<std::string> & params)
{
    sqlite3_stmt* stmt;
    if(sqlite3_prepare_v2(db, query.c_str(), -1, & stmt, nullptr) == SQLITE_OK)
    {
        for(size_t i = 0; i < params.size(); ++i)
        {
            sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
        }

        if(sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::cerr << ERR_MSG_FAILED_QUERY << ": " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            return false;
        }

        sqlite3_finalize(stmt);
        return true;
    }
    else
    {
        std::cerr << ERR_MSG_FAILED_PREPARE_STMT << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

/**
 * @brief Executes an SQL query and returns the number of affected rows.
 * @param query The SQL query to execute.
 * @return The number of affected rows, or -1 if an error occurred.
 */
int SQLiteDatabase::executeWithRowCount(const std::string& query)
{
    char* errMsg = nullptr;
    if(sqlite3_exec(db, query.c_str(), nullptr, nullptr, & errMsg) != SQLITE_OK)
    {
        std::cerr << ERR_MSG_SQL_ERR << errMsg << std::endl;
        sqlite3_free(errMsg);
        return -1;
    }
    return sqlite3_changes(db);
}

/**
 * @brief Begins a transaction.
 * @return True if the transaction was started successfully, false otherwise.
 */
bool SQLiteDatabase::beginTransaction()
{
    return execute("BEGIN TRANSACTION;");
}

/**
 * @brief Commits the current transaction.
 * @return True if the transaction was committed successfully, false otherwise.
 */
bool SQLiteDatabase::commitTransaction()
{
    return execute("COMMIT;");
}

/**
 * @brief Rolls back the current transaction.
 * @return True if the transaction was rolled back successfully, false otherwise.
 */
bool SQLiteDatabase::rollbackTransaction()
{
    return execute("ROLLBACK;");
}

/**
 * @brief Gets the last error message.
 * @return The last error message as a string.
 */
std::string SQLiteDatabase::getLastError() const
{
    if(db)
    {
        return sqlite3_errmsg(db);
    }
    return "Database is not connected.";
}

/**
 * @brief Reconnects to the database.
 * @throws std::runtime_error if reconnection fails.
 */
void SQLiteDatabase::reconnect()
{
    disconnect();
    if(!connect(dbPath))
    {
        throw std::runtime_error(ERR_MSG_FAILED_RECONNECT_DB);
    }
}

/**
 * @brief Gets the type of the database.
 * @return The database type (SQLite in this case).
 */
DataBaseType SQLiteDatabase::getDatabaseType() const
{
    return dbType;
}
