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

#ifndef DATABASE_INTERFACE_H
#define DATABASE_INTERFACE_H

#include <string>
#include <vector>
#include <map>
#include "log_entry.h"
#include "database_helper.h"

/**
 * @class IDatabase
 * @brief Interface for database operations.
 */
class IDatabase
{
    public:
        virtual ~IDatabase() = default;

        /**
         * @brief Connects to the database.
         * @param connectionString The connection string or parameters for the database.
         * @return True if the connection was successful, false otherwise.
         */
        virtual bool connect(const std::string& connectionString) = 0;

        /**
         * @brief Disconnects from the database.
         */
        virtual void disconnect() = 0;

        /**
         * @brief Checks if the database is connected.
         * @return True if the database is connected, false otherwise.
         */
        virtual bool isConnected() const = 0;

        /**
         * @brief Executes an SQL query.
         * @param query The SQL query to execute.
         * @return True if the query was executed successfully, false otherwise.
         */
        virtual bool execute(const std::string& query) = 0;

        /**
         * @brief Executes an SQL query and returns the result.
         * @param query The SQL query to execute.
         * @return A vector of maps representing the query result. Each map contains key-value pairs of column names and their corresponding values.
         */
        virtual std::vector<std::map<std::string, std::string>> query(const std::string& query) = 0;

        /**
         * @brief Prepares and executes an SQL query with parameters.
         * @param query The SQL query to execute.
         * @param params The parameters to bind to the query.
         * @return True if the query was executed successfully, false otherwise.
         */
        virtual bool executeWithParams(const std::string& query, const std::vector<std::string> & params) = 0;

        /**
         * @brief Executes an SQL query and returns the number of affected rows.
         * @param query The SQL query to execute.
         * @return The number of affected rows, or -1 if an error occurred.
         */
        virtual int executeWithRowCount(const std::string& query) = 0;

        /**
         * @brief Begins a transaction.
         * @return True if the transaction was started successfully, false otherwise.
         */
        virtual bool beginTransaction() = 0;

        /**
         * @brief Commits the current transaction.
         * @return True if the transaction was committed successfully, false otherwise.
         */
        virtual bool commitTransaction() = 0;

        /**
         * @brief Rolls back the current transaction.
         * @return True if the transaction was rolled back successfully, false otherwise.
         */
        virtual bool rollbackTransaction() = 0;

        /**
        * @brief Drops the database if it exists.
        * @return True if the database was successfully dropped, false otherwise.
        */
        virtual bool dropDatabaseIfExists(const std::string& connectionString) = 0;

        /**
         * @brief Gets the last error message.
         * @return The last error message as a string.
         */
        virtual std::string getLastError() const = 0;

        /**
         * @brief Gets the type of the database.
         * @return The database type.
         */
        virtual DataBaseType getDatabaseType() const = 0;
};

#endif // DATABASE_INTERFACE_H