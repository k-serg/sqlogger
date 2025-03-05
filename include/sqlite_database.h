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

#ifndef SQLITE_DATABASE_H
#define SQLITE_DATABASE_H

#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <vector>
#include "database_interface.h"

#define USE_WAL_MODE 1

/**
 * @class SQLiteDatabase
 * @brief Implementation of the IDatabase interface for SQLite databases.
 */
class SQLiteDatabase : public IDatabase
{
    public:
        /**
         * @brief Constructs an SQLiteDatabase object.
         * @param dbPath The path to the SQLite database file.
         * @throws std::runtime_error if the database cannot be opened.
         */
        SQLiteDatabase(const std::string& dbPath);

        /**
         * @brief Destructor for SQLiteDatabase.
         * Closes the database connection.
         */
        ~SQLiteDatabase();

        /**
         * @brief Executes an SQL query.
         * @param query The SQL query to execute.
         * @return True if the query was executed successfully, false otherwise.
         */
        bool execute(const std::string& query) override;

        /**
         * @brief Executes an SQL query and returns the result.
         * @param query The SQL query to execute.
         * @return A vector of maps representing the query result.
         */
        std::vector<std::map<std::string, std::string>> query(const std::string& query) override;

        /**
         * @brief Prepares and executes an SQL query with parameters.
         * @param query The SQL query to execute.
         * @param params The parameters to bind to the query.
         * @return True if the query was executed successfully, false otherwise.
         */
        bool executeWithParams(const std::string& query, const std::vector<std::string> & params) override;

    private:
        sqlite3* db; /**< SQLite database handle. */
        std::string dbPath; /**< Path to the database file. */

        /**
         * @brief Reconnects to the database.
         * @throws std::runtime_error if reconnection fails.
         */
        void reconnect();
};

#endif // SQLITE_DATABASE_H