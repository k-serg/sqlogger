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

#ifndef POSTGRESQL_DATABASE_H
#define POSTGRESQL_DATABASE_H

#include <sstream>
#include <iostream>
#include <libpq-fe.h>
#include "sqlogger/database/database_interface.h"

/**
 * @class PostgreSQLDatabase
 * @brief PostgreSQL-specific implementation of IDatabase interface
 *
 * Handles all PostgreSQL-specific database operations including connection management,
 * transaction control, and query execution. Compatible with SQLogger's logging system.
 */
class PostgreSQLDatabase : public IDatabase
{
    public:
        /**
         * @brief Construct a new PostgreSQL Database object
         * @param connectionString Connection string in format "host=... user=... password=... dbname=..."
         * @throws std::runtime_error if connection or database creation fails
         */
        explicit PostgreSQLDatabase(const std::string& connectionString);

        /**
         * @brief Destroy the PostgreSQL Database object
         *
         * Automatically rolls back any active transaction and closes the connection
         */
        virtual ~PostgreSQLDatabase();

        /**
         * @brief Establish connection to PostgreSQL server
         * @param connectionString Connection parameters
         * @return true if connection succeeded
         * @see IDatabase::connect()
         */
        bool connect(const std::string& connectionString) override;

        /**
         * @brief Close database connection
         * @see IDatabase::disconnect()
         */
        void disconnect() override;

        /**
         * @brief Check connection status
         * @return true if connection is active
         * @see IDatabase::isConnected()
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
         * @brief Begin database transaction
         * @return true if transaction started successfully
         * @see IDatabase::beginTransaction()
         */
        bool beginTransaction() override;

        /**
         * @brief Commit current transaction
         * @return true if transaction committed successfully
         * @see IDatabase::commitTransaction()
         */
        bool commitTransaction() override;

        /**
         * @brief Roll back current transaction
         * @return true if transaction rolled back successfully
         * @see IDatabase::rollbackTransaction()
         */
        bool rollbackTransaction() override;

        /**
         * @brief Drop database if exists (admin operation)
         * @param connectionString Original connection string
         * @return true if operation succeeded
         * @see IDatabase::dropDatabaseIfExists()
         */
        bool dropDatabaseIfExists(const std::string& connectionString) override;

        /**
         * @brief Get last error message
         * @return Error description string
         * @see IDatabase::getLastError()
         */
        std::string getLastError() const override;

        /**
         * @brief Get database type
         * @return DataBaseType::PostgreSQL
         * @see IDatabase::getDatabaseType()
         */
        DataBaseType getDatabaseType() const override;

    private:

        /**
         * @brief Create database if not exists
         * @param connectionString Connection parameters
         * @return true if database exists or was created
         */
        bool createDatabaseIfNotExists(const std::string& connectionString);

        /**
         * @brief Parse connection string into key-value pairs
         * @param connectionString Space-separated key=value pairs
         * @return Map of connection parameters
         */
        std::map<std::string, std::string> parseConnectionString(const std::string& connectionString);

        PGconn* conn;                       ///< PostgreSQL connection handle
        std::string lastError;              ///< Last error message storage
        const bool allowCreateDB;           ///< Database creation allowed flag
        bool transactionInProgress;         ///< Active transaction state flag
        const DataBaseType dbType = DataBaseType::PostgreSQL; /**< The type of the database (PostgreSQL). */
};

#endif // POSTGRESQL_DATABASE_H