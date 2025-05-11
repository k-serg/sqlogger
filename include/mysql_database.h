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

#ifndef MYSQL_DATABASE_H
#define MYSQL_DATABASE_H

#include <mysql.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "database_interface.h"
#include "database_helper.h"
#include "log_config.h"
#include "log_strings.h"

/**
 * @class MySQLDatabase
 * @brief Implementation of the IDatabase interface for MySQL databases.
 */
class MySQLDatabase : public IDatabase
{
    public:
        /**
         * @brief Constructs a MySQLDatabase object.
         * @param connectionString The connection string in the format "host=...;user=...;password=...;db=...".
         * @throws std::runtime_error if the MySQL library cannot be initialized.
         */
        MySQLDatabase(const std::string& connectionString);

        /**
         * @brief Destructor for MySQLDatabase.
         * Closes the database connection.
         */
        ~MySQLDatabase();

        /**
         * @brief Connects to the MySQL database using the provided connection string.
         * @param connectionString The connection string in the format "host=...;user=...;password=...;database=...".
         * @return True if the connection was successful, false otherwise.
         */
        bool connect(const std::string& connectionString) override;

        /**
         * @brief Disconnects from the MySQL database.
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
        bool dropDatabaseIfExists(const std::string& connectionString) override;

        /**
         * @brief Gets the last error message.
         * @return The last error message as a string.
         */
        std::string getLastError() const override;

        /**
         * @brief Gets the type of the database.
         * @return The database type (MySQL in this case).
         */
        DataBaseType getDatabaseType() const override;

    private:
        MYSQL* conn; /**< MySQL connection handle. */

        /**
         * @brief Parses a MySQL connection string.
         * @param connectionString The connection string in the format "host=...;user=...;password=...;database=...".
         * @return A map containing the connection parameters.
         */
        std::map<std::string, std::string> parseConnectionString(const std::string& connectionString);

        /**
        * @brief Creates a database if it does not already exist.
        * @param connectionString The connection string containing database parameters.
        * @return True if the database was created or already exists, false otherwise.
        */
        bool createDatabaseIfNotExists(const std::string& connectionString);

        const DataBaseType dbType = DataBaseType::MySQL; /**< The type of the database (MySQL). */
        std::string lastError;  /**< Last error message storage*/
        bool allowCreateDB; /**< Allow create database on server (execute CREATE DATABASE). */
};

#endif // MYSQL_DATABASE_H