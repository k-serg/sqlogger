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
 * @brief Constructor for SQLiteDatabase
 * @param dbPath The path to the SQLite database file
 */
SQLiteDatabase::SQLiteDatabase(const std::string& dbPath) : dbPath(dbPath)
{
    std::string errMsg;
    if(!FSHelper::CreateDir(dbPath, errMsg))
    {
        throw std::runtime_error(ERR_MSG_FAILED_CREATE_DIR + errMsg);
    }

    const std::string utf8dbPath = std::filesystem::path(dbPath).u8string();

    if(sqlite3_open(utf8dbPath.c_str(), & db) != SQLITE_OK)
    {
        throw std::runtime_error(ERR_MSG_FAILED_OPEN_DB + dbPath);
    }

#if USE_WAL_MODE == 1
    if(!execute("PRAGMA journal_mode=WAL;"))
    {
        throw std::runtime_error(ERR_MSG_FAILED_ENABLE_WAL);
    }
#endif

}

/**
 * @brief Destructor for SQLiteDatabase
 */
SQLiteDatabase::~SQLiteDatabase()
{
    sqlite3_close(db);
}

/**
 * @brief Execute an SQL query
 * @param query The SQL query to execute
 * @return True if the query was executed successfully, false otherwise
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
 * @brief Execute an SQL query and return the result
 * @param query The SQL query to execute
 * @return A vector of maps representing the query result
 */
std::vector<std::map<std::string, std::string>> SQLiteDatabase::query(const std::string& query)
{
    std::vector<std::map<std::string, std::string>> result;
    sqlite3_stmt* stmt;

    // Prepare the SQL statement
    if(sqlite3_prepare_v2(db, query.c_str(), -1, & stmt, nullptr) == SQLITE_OK)
    {
        // Fetch rows one by one
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            std::map<std::string, std::string> row;
            int columnCount = sqlite3_column_count(stmt);

            // Iterate through columns
            for(int i = 0; i < columnCount; ++i)
            {
                const char* columnName = sqlite3_column_name(stmt, i); // Get column name
                const char* columnValue = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i)); // Get column value

                // Add column name and value to the row
                row[columnName] = columnValue ? columnValue : "";
            }

            // Add the row to the result
            result.push_back(row);
        }

        // Finalize the statement
        sqlite3_finalize(stmt);
    }
    else
    {
        // Log an error if the query fails
        std::cerr << ERR_MSG_FAILED_QUERY << ": " << sqlite3_errmsg(db) << std::endl;
    }

    return result;
}

/**
 * @brief Prepare and execute an SQL query with parameters
 * @param query The SQL query to execute
 * @param params The parameters to bind to the query
 * @return True if the query was executed successfully, false otherwise
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
 * @brief Reconnect to the database
 */
void SQLiteDatabase::reconnect()
{
    sqlite3_close(db);
    if(sqlite3_open(dbPath.c_str(), & db) != SQLITE_OK)
    {
        throw std::runtime_error(ERR_MSG_FAILED_RECONNECT_DB);
    }

#if USE_WAL_MODE == 1
    if(!execute("PRAGMA journal_mode=WAL;"))
    {
        throw std::runtime_error(ERR_MSG_FAILED_ENABLE_WAL);
    }
#endif
}
