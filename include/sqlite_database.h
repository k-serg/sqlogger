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
#include <map>
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
         * @brief Connects to the SQLite database.
         * @param path The path to the SQLite database file.
         * @return True if the connection was successful, false otherwise.
         */
        bool connect(const std::string& path) override;

        /**
         * @brief Disconnects from the SQLite database.
         */
        void disconnect() override;

        /**
         * @brief Checks if the database is connected.
         * @return True if the database is connected, false otherwise.
         */
        bool isConnected() const override;

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
        bool execute(
            const std::string& query,
            const std::vector<std::string> & params = {},
            int* affectedRows = nullptr) override;

        /**
        * @brief Executes an SQL query and returns the result.
        * @param query The SQL query to execute.
        * @param params The parameters to bind to the query.
        * @return A vector of maps representing the query result.
        */
        std::vector<std::map<std::string, std::string>> query(const std::string& query,
                const std::vector<std::string> & params = {}) override;

        /**
         * @brief Begins a transaction.
         * @return True if the transaction was started successfully, false otherwise.
         */
        bool beginTransaction() override;

        /**
         * @brief Commits the current transaction.
         * @return True if the transaction was committed successfully, false otherwise.
         */
        bool commitTransaction() override;

        /**
         * @brief Rolls back the current transaction.
         * @return True if the transaction was rolled back successfully, false otherwise.
         */
        bool rollbackTransaction() override;

        /**
         * @brief Gets the last error message.
         * @return The last error message as a string.
         */
        std::string getLastError() const override;

        /**
        * @brief Drops the database if it exists.
        * @return True if the database was successfully dropped, false otherwise.
        */
        bool dropDatabaseIfExists(const std::string& dbName) override
        {
            return false;
        }; // TODO: Implement

        DataBaseType getDatabaseType() const override;

    private:
        /**
         * @brief Reconnects to the database.
         * @throws std::runtime_error if reconnection fails.
         */
        void reconnect();

        /**
        * @brief Creates a database if it does not already exist.
        * @param dbPath The path to the SQLite database file.
        * @return True if the database was created or already exists, false otherwise.
        */
        bool createDatabaseIfNotExists(const std::string& dbPath);

        sqlite3* db; /**< SQLite database handle. */
        std::string dbPath; /**< Path to the database file. */
        const DataBaseType dbType = DataBaseType::SQLite; /**< The type of the database (SQLite). */
};

#endif // SQLITE_DATABASE_H