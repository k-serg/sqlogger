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

#ifndef MOCK_DATABASE_H
#define MOCK_DATABASE_H

#include <vector>
#include <map>
#include <string>
#include <mutex>
#include <functional>
#include "log_entry.h"
#include "database_interface.h"

/**
 * @class MockDatabase
 * @brief Mock implementation of the IDatabase interface for testing.
 */
class MockDatabase : public IDatabase
{
    public:
        /**
         * @brief Connects to the database.
         * @param connectionString The connection string or parameters for the database.
         * @return True if the connection was successful, false otherwise.
         */
        bool connect(const std::string& connectionString) override;

        /**
         * @brief Disconnects from the database.
         */
        void disconnect() override;

        /**
         * @brief Checks if the database is connected.
         * @return True if the database is connected, false otherwise.
         */
        bool isConnected() const override;

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

        /**
         * @brief Executes an SQL query and returns the number of affected rows.
         * @param query The SQL query to execute.
         * @return The number of affected rows, or -1 if an error occurred.
         */
        int executeWithRowCount(const std::string& query) override;

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
        bool dropDatabaseIfExists(const std::string& dbName) override;

        /**
         * @brief Gets the type of the database.
         * @return The database type.
         */
        DataBaseType getDatabaseType() const override;

        /**
         * @brief Override for executeWithParams to simulate custom behavior.
         */
        std::function<bool(const std::string&, const std::vector<std::string> &)> executeWithParamsOverride;

        /**
         * @brief Clears mock data.
         */
        void clearMockData();

        /**
         * @brief Gets the list of executed queries.
         * @return The list of executed queries.
         */
        std::vector<std::string> getExecutedQueries() const;

        /**
         * @brief Gets the list of executed parameters.
         * @return The list of executed parameters.
         */
        std::vector<std::vector<std::string>> getExecutedParams() const;

        /**
         * @brief Gets the mock data.
         * @return The mock data.
         */
        std::vector<std::map<std::string, std::string>> getMockData() const;

    private:
        /**
         * @struct Filter
         * @brief Represents a filter for querying mock data.
         */
        struct Filter
        {
            std::string field; /**< The field to filter on. */
            std::string op;    /**< The operator for the filter (e.g., "=", ">="). */
            std::string value; /**< The value to compare against. */
            std::string type;  /**< The type of the field. */

            /**
             * @brief Determines the type of the field.
             * @return The type of the field as a string.
             */
            std::string fieldToType() const;
        };

        /**
         * @brief Parses a filter from a query string.
         * @param filterStr The filter string to parse.
         * @param filters The vector of filters to populate.
         */
        void parseFilter(const std::string& filterStr, std::vector<Filter> & filters);

        /**
         * @brief Applies a filter to a value.
         * @param value The value to apply the filter to.
         * @param filter The filter to apply.
         * @return True if the value matches the filter, false otherwise.
         */
        bool applyFilter(const std::string& value, const Filter& filter);

        std::vector<std::string> executedQueries; /**< List of executed queries. */
        std::vector<std::vector<std::string>> executedParams; /**< List of executed parameters. */
        std::vector<std::map<std::string, std::string>> mockData; /**< Mock data for testing. */
        mutable std::mutex mutex; /**< Mutex for thread safety. */
        DataBaseType dbType = DataBaseType::Mock; /**< The type of the database (Mock). */
};

#endif // MOCK_DATABASE_H