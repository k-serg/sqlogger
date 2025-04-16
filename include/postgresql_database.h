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

#include "database_interface.h"
#include <libpq-fe.h>

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
         * @brief Execute SQL query without results
         * @param query SQL query to execute
         * @return true if query succeeded
         * @see IDatabase::execute()
         */
        bool execute(const std::string& query) override;

        /**
         * @brief Execute SQL query with results
         * @param query SQL query to execute
         * @return Vector of result rows as column-value maps
         * @see IDatabase::query()
         */
        std::vector<std::map<std::string, std::string>> query(const std::string& query) override;

        /**
         * @brief Execute parameterized SQL query
         * @param query SQL query with placeholders
         * @param params Parameter values
         * @return true if query succeeded
         * @see IDatabase::executeWithParams()
         */
        bool executeWithParams(const std::string& query,
                               const std::vector<std::string> & params) override;

        /**
         * @brief Execute SQL query and get affected row count
         * @param query SQL query to execute
         * @return Number of affected rows or -1 on error
         * @see IDatabase::executeWithRowCount()
         */
        int executeWithRowCount(const std::string& query) override;

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
        std::map<std::string, std::string> parseConnectionString(
            const std::string& connectionString);

        PGconn* conn;                       ///< PostgreSQL connection handle
        std::string lastError;              ///< Last error message storage
        const bool allowCreateDB;           ///< Database creation allowed flag
        bool transactionInProgress;         ///< Active transaction state flag
        const DataBaseType dbType = DataBaseType::PostgreSQL; /**< The type of the database (PostgreSQL). */
};

#endif // POSTGRESQL_DATABASE_H